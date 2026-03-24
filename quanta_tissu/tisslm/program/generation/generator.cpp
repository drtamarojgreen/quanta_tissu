#include "generator.h"
#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include "quanta_tissu/tisslm/program/retrieval/retrieval_strategy.h"
#include <random>
#include <algorithm>
#include <limits>

namespace TissLM {
namespace Generation {

using namespace TissLM::Core;
using namespace TissNum;
using namespace Retrieval;

namespace {
    TissNum::Matrix softmax(const TissNum::Matrix& input) {
        TissNum::Matrix output({input.rows(), input.cols()});
        for (size_t r = 0; r < input.rows(); ++r) {
            float max_val = -std::numeric_limits<float>::infinity();
            for (size_t c = 0; c < input.cols(); ++c) if (input({r, c}) > max_val) max_val = input({r, c});
            float sum_exp = 0.0f;
            for (size_t c = 0; c < input.cols(); ++c) {
                output({r, c}) = std::exp(input({r, c}) - max_val);
                sum_exp += output({r, c});
            }
            for (size_t c = 0; c < input.cols(); ++c) output({r, c}) /= sum_exp;
        }
        return output;
    }
}

Generator::Generator(std::shared_ptr<TissLM::Core::Model> model, const GenerationConfig& config)
    : model_(model), config_(config), gen_(config.seed.has_value() ? config.seed.value() : std::random_device()()) {}

Generator::Generator(std::shared_ptr<TissLM::Core::Model> model, std::shared_ptr<TissLM::Core::Model> draft, const GenerationConfig& config)
    : model_(model), draft_model_(draft), config_(config), gen_(config.seed.has_value() ? config.seed.value() : std::random_device()()) {}

std::vector<int> Generator::generate(const std::vector<int>& prompt_tokens, int max_new_tokens) {
    std::vector<int> res = prompt_tokens;
    std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> kv;
    auto* tm = static_cast<TransformerModel*>(model_.get());
    TissNum::Matrix next_logits;
    if (!prompt_tokens.empty()) {
        TissNum::Matrix pm({1, prompt_tokens.size()});
        for (size_t i = 0; i < prompt_tokens.size(); ++i) pm({0, i}) = (float)prompt_tokens[i];
        std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> nkv;
        TissNum::Matrix al = tm->forward_inference(pm, kv, nkv); kv = nkv;
        next_logits = TissNum::Matrix({1, al.cols()});
        for (size_t c = 0; c < al.cols(); ++c) next_logits({0, c}) = al({al.rows() - 1, c});
    } else next_logits = TissNum::Matrix({1, (size_t)model_->get_vocab_size()});

    for (int i = 0; i < max_new_tokens; ++i) {
        int token = sample_token(next_logits, res, i);
        if (std::find(config_.eos_ids.begin(), config_.eos_ids.end(), token) != config_.eos_ids.end()) {
            if (!config_.suppress_eos) res.push_back(token);
            break;
        }
        res.push_back(token);
        TissNum::Matrix it({1, 1}); it({0, 0}) = (float)token;
        std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> nkv;
        next_logits = tm->forward_inference(it, kv, nkv); kv = nkv;
    }
    return res;
}

int Generator::sample_token(const TissNum::Matrix& logits, const std::vector<int>& past, int step) {
    TissNum::Matrix pl = logits;
    for (const auto& b : config_.logit_bias) if (b.first < pl.cols()) pl({0, (size_t)b.first}) += b.second;
    float temp = config_.temperature;
    if (!config_.temperature_schedule.empty()) temp = config_.temperature_schedule[std::min((size_t)step, config_.temperature_schedule.size() - 1)];
    if (temp == 0.0f || config_.method == "greedy") {
        int mi = -1; float ml = -1e9;
        for (size_t c = 0; c < pl.cols(); ++c) if (pl({0, c}) > ml) { ml = pl({0, c}); mi = c; }
        return mi;
    }
    pl = pl / temp;
    if (config_.repetition_penalty != 1.0f) {
        for (int tid : past) if (tid < (int)pl.cols()) {
            if (pl({0, (size_t)tid}) > 0) pl({0, (size_t)tid}) /= config_.repetition_penalty;
            else pl({0, (size_t)tid}) *= config_.repetition_penalty;
        }
    }
    if (config_.no_repeat_ngram_size > 0 && past.size() >= config_.no_repeat_ngram_size) {
        std::vector<int> pref(past.end() - (config_.no_repeat_ngram_size - 1), past.end());
        for (size_t i = 0; i <= past.size() - config_.no_repeat_ngram_size; ++i) {
            bool m = true; for (size_t j = 0; j < pref.size(); ++j) if (past[i + j] != pref[j]) { m = false; break; }
            if (m) { int bt = past[i + config_.no_repeat_ngram_size - 1]; if (bt < (int)pl.cols()) pl({0, (size_t)bt}) = -1e9; }
        }
    }
    if (config_.method == "bayesian_influenced") {
        auto* tm = static_cast<TransformerModel*>(model_.get());
        auto ve = tm->get_embeddings_as_vectors();
        BayesianSimilarityStrategy s; std::map<std::string, std::any> k; k["eigenvalues"] = config_.eigenvalues; k["rng"] = &gen_;
        auto ss = s.calculate_similarity(config_.query_embedding, ve, k);
        for (size_t i = 0; i < ss.size(); ++i) if (i < pl.cols()) pl({0, i}) += ss[i] * config_.bayesian_influence_scale;
    }
    TissNum::Matrix probs = softmax(pl);
    std::vector<std::pair<float, int>> tp; for (size_t c = 0; c < probs.cols(); ++c) tp.push_back({probs({0, c}), (int)c});
    std::sort(tp.rbegin(), tp.rend());
    if (config_.method == "top_a" && config_.top_a > 0.0f) {
        float pm = tp[0].first; size_t li = 0;
        for (size_t i = 0; i < tp.size(); ++i) { if (tp[i].first < config_.top_a * pm) break; li = i; }
        tp.resize(li + 1);
    } else if (config_.method == "nucleus" && config_.top_p.has_value() && config_.top_p.value() < 1.0f) {
        float cp = 0.0f; size_t li = 0;
        for (size_t i = 0; i < tp.size(); ++i) { cp += tp[i].first; li = i; if (cp >= config_.top_p.value()) break; }
        tp.resize(li + 1);
    } else if (config_.method == "top_k" && config_.top_k.has_value() && config_.top_k.value() > 0 && config_.top_k.value() < (int)tp.size()) tp.resize(config_.top_k.value());
    float tot = 0.0f; for (const auto& p : tp) tot += p.first;
    std::vector<float> fp; std::vector<int> ft; for (const auto& p : tp) { fp.push_back(p.first / tot); ft.push_back(p.second); }
    std::discrete_distribution<> d(fp.begin(), fp.end()); return ft[d(gen_)];
}

std::vector<int> Generator::beam_search(const std::vector<int>& prompt, int n, int w, int eos) {
    std::vector<std::pair<std::vector<int>, float>> b; b.push_back({prompt, 0.0f});
    for (int i = 0; i < n; ++i) {
        std::vector<std::pair<std::vector<int>, float>> nb;
        for (const auto& beam : b) {
            if (beam.first.back() == eos) { nb.push_back(beam); continue; }
            TissNum::Matrix pm({1, beam.first.size()}); for (size_t j = 0; j < beam.first.size(); ++j) pm({0, j}) = (float)beam.first[j];
            TissNum::Matrix l = model_->forward(pm); TissNum::Matrix last({1, l.cols()}); for (size_t c = 0; c < l.cols(); ++c) last({0, c}) = l({l.rows() - 1, c});
            TissNum::Matrix probs = softmax(last);
            std::vector<std::pair<float, int>> tp; for (size_t c = 0; c < probs.cols(); ++c) tp.push_back({probs({0, c}), (int)c});
            std::sort(tp.rbegin(), tp.rend());
            for (int j = 0; j < w && j < (int)tp.size(); ++j) {
                std::vector<int> nseq = beam.first; nseq.push_back(tp[j].second);
                nb.push_back({nseq, beam.second + std::log(tp[j].first)});
            }
        }
        std::sort(nb.begin(), nb.end(), [](const auto& x, const auto& y) { return x.second > y.second; });
        b.assign(nb.begin(), nb.begin() + std::min((int)nb.size(), w));
    }
    return b[0].first;
}

std::vector<int> Generator::contrastive_search(const std::vector<int>& prompt, int n, int w, float a, int eos) {
    std::vector<int> curr = prompt; auto* tm = static_cast<TransformerModel*>(model_.get());
    for (int i = 0; i < n; ++i) {
        TissNum::Matrix pm({1, curr.size()}); for (size_t j = 0; j < curr.size(); ++j) pm({0, j}) = (float)curr[j];
        TissNum::Matrix l = model_->forward(pm); TissNum::Matrix last({1, l.cols()}); for (size_t c = 0; c < l.cols(); ++c) last({0, c}) = l({l.rows() - 1, c});
        TissNum::Matrix p = softmax(last);
        std::vector<std::pair<float, int>> tp; for (size_t c = 0; c < p.cols(); ++c) tp.push_back({p({0, c}), (int)c});
        std::sort(tp.rbegin(), tp.rend());
        std::vector<int> top; for (int j = 0; j < w && j < (int)tp.size(); ++j) top.push_back(tp[j].second);
        const auto& em = tm->get_embeddings(); TissNum::Matrix ce({curr.size(), em.cols()});
        for (size_t j = 0; j < curr.size(); ++j) for (size_t k = 0; k < em.cols(); ++k) ce({j, k}) = em({(size_t)curr[j], k});
        int best = -1; float ms = -1e9;
        for (int tid : top) {
            float conf = p({0, (size_t)tid}); TissNum::Matrix cand({1, em.cols()}); for (size_t k = 0; k < em.cols(); ++k) cand({0, k}) = em({(size_t)tid, k});
            float msim = -1.0f;
            for (size_t j = 0; j < ce.rows(); ++j) {
                float d = 0, na = 0, nb = 0; for (size_t k = 0; k < ce.cols(); ++k) { d += ce({j, k}) * cand({0, k}); na += ce({j, k}) * ce({j, k}); nb += cand({0, k}) * cand({0, k}); }
                msim = std::max(msim, d / (std::sqrt(na) * std::sqrt(nb)));
            }
            float s = (1 - a) * conf - a * msim; if (s > ms) { ms = s; best = tid; }
        }
        if (best == eos) break; curr.push_back(best);
    }
    return curr;
}

std::vector<int> Generator::mirostat_sampling(const std::vector<int>& prompt, int n, float tau, float eta, int eos) {
    std::vector<int> curr = prompt; float ms = 2 * tau;
    for (int i = 0; i < n; ++i) {
        TissNum::Matrix pm({1, curr.size()}); for (size_t j = 0; j < curr.size(); ++j) pm({0, j}) = (float)curr[j];
        TissNum::Matrix l = model_->forward(pm); TissNum::Matrix last({1, l.cols()}); for (size_t c = 0; c < l.cols(); ++c) last({0, c}) = l({l.rows() - 1, c});
        TissNum::Matrix p = softmax(last);
        std::vector<std::pair<float, int>> tp; for (size_t c = 0; c < p.cols(); ++c) tp.push_back({p({0, c}), (int)c});
        std::sort(tp.rbegin(), tp.rend());
        int k = 0; for (; k < (int)tp.size(); ++k) if (-std::log2(tp[k].first) > ms) break;
        if (k == 0) k = tp.size();
        std::vector<int> ni; float snp = 0; for (int j = 0; j < k; ++j) { ni.push_back(tp[j].second); snp += tp[j].first; }
        std::vector<float> np; for (int j = 0; j < k; ++j) np.push_back(tp[j].first / snp);
        std::discrete_distribution<> d(np.begin(), np.end()); int tok = ni[d(gen_)];
        ms -= eta * (-std::log2(p({0, (size_t)tok})) - tau);
        if (tok == eos) break; curr.push_back(tok);
    }
    return curr;
}

std::vector<int> Generator::speculative_sampling(const std::vector<int>& prompt, int n) {
    if (!draft_model_) return generate(prompt, n);
    std::vector<int> res = prompt; auto* mtm = static_cast<TransformerModel*>(model_.get()); auto* dtm = static_cast<TransformerModel*>(draft_model_.get());
    for (int i = 0; i < n; ) {
        std::vector<int> ds; std::vector<int> cs = res;
        for (int k = 0; k < 5; ++k) {
            TissNum::Matrix in({1, cs.size()}); for (size_t j = 0; j < cs.size(); ++j) in({0, j}) = (float)cs[j];
            TissNum::Matrix l = dtm->forward(in); TissNum::Matrix last({1, l.cols()}); for (size_t c = 0; c < l.cols(); ++c) last({0, c}) = l({l.rows() - 1, c});
            int nt = sample_token(last, cs, i + k); ds.push_back(nt); cs.push_back(nt);
        }
        TissNum::Matrix min({1, res.size() + 5}); for (size_t j = 0; j < res.size(); ++j) min({0, j}) = (float)res[j];
        for (int k = 0; k < 5; ++k) min({0, res.size() + k}) = (float)ds[k];
        TissNum::Matrix ml = mtm->forward(min);
        int acc = 0;
        for (int k = 0; k < 5; ++k) {
            TissNum::Matrix dtl({1, ml.cols()}); for (size_t c = 0; c < ml.cols(); ++c) dtl({0, c}) = ml({res.size() - 1 + k, c});
            int mmt = sample_token(dtl, res, i + k);
            if (ds[k] == mmt) { res.push_back(ds[k]); acc++; }
            else { res.push_back(mmt); acc++; break; }
        }
        i += acc;
    }
    return res;
}

std::vector<std::vector<int>> Generator::generate_batch(const std::vector<std::vector<int>>& prompts, int n) {
    std::vector<std::vector<int>> res; for (const auto& p : prompts) res.push_back(generate(p, n)); return res;
}

}
}

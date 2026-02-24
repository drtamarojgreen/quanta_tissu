#include "stellar_generator.h"
#include "core/transformer_model.h"
#include "retrieval/retrieval_strategy.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace TissLM {
namespace Stellar {

using namespace TissLM::Core;
using namespace TissNum;

namespace {
    /**
     * @brief Computes the softmax of a matrix (typically logits).
     */
    Matrix softmax(const Matrix& input) {
        Matrix output({input.rows(), input.cols()});
        for (size_t r = 0; r < input.rows(); ++r) {
            float max_val = -std::numeric_limits<float>::infinity();
            for (size_t c = 0; c < input.cols(); ++c) {
                if (input({r, c}) > max_val) max_val = input({r, c});
            }

            float sum_exp = 0.0f;
            for (size_t c = 0; c < input.cols(); ++c) {
                output({r, c}) = std::exp(input({r, c}) - max_val);
                sum_exp += output({r, c});
            }

            for (size_t c = 0; c < input.cols(); ++c) {
                output({r, c}) /= (sum_exp + 1e-9f);
            }
        }
        return output;
    }
}

StellarGenerator::StellarGenerator(
    std::shared_ptr<TissLM::Core::Model> model,
    const TissLM::Generation::GenerationConfig& config
) : model_(model), config_(config), gen_(config.seed.has_value() ? config.seed.value() : std::random_device()()) {
}

StellarGenerator::StellarGenerator(
    std::shared_ptr<TissLM::Core::Model> model,
    std::shared_ptr<TissLM::Core::Model> draft_model,
    const TissLM::Generation::GenerationConfig& config
) : model_(model), draft_model_(draft_model), config_(config), gen_(config.seed.has_value() ? config.seed.value() : std::random_device()()) {
}

std::vector<int> StellarGenerator::generate(const std::vector<int>& prompt_tokens, int max_new_tokens) {
    std::vector<int> sequence = prompt_tokens;
    std::vector<std::pair<Matrix, Matrix>> kv_cache;
    auto transformer = static_cast<TransformerModel*>(model_.get());

    Matrix logits;

    // Process prompt
    if (!prompt_tokens.empty()) {
        Matrix prompt_matrix({1, prompt_tokens.size()});
        for (size_t i = 0; i < prompt_tokens.size(); ++i) prompt_matrix({0, i}) = static_cast<float>(prompt_tokens[i]);

        std::vector<std::pair<Matrix, Matrix>> next_cache;
        Matrix all_logits = transformer->forward_inference(prompt_matrix, {}, next_cache);
        kv_cache = next_cache;

        logits = Matrix({1, all_logits.cols()});
        for (size_t c = 0; c < all_logits.cols(); ++c) logits({0, c}) = all_logits({all_logits.rows() - 1, c});
    } else {
        logits = Matrix({1, (size_t)model_->get_vocab_size()});
    }

    for (int i = 0; i < max_new_tokens; ++i) {
        int token = sample_token(logits, sequence, i);

        bool is_eos = std::find(config_.eos_ids.begin(), config_.eos_ids.end(), token) != config_.eos_ids.end();
        if (is_eos) {
            if (!config_.suppress_eos) sequence.push_back(token);
            break;
        }
        sequence.push_back(token);

        Matrix input({1, 1});
        input({0, 0}) = static_cast<float>(token);

        std::vector<std::pair<Matrix, Matrix>> next_cache;
        logits = transformer->forward_inference(input, kv_cache, next_cache);
        kv_cache = next_cache;
    }

    return sequence;
}

std::vector<int> StellarGenerator::beam_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, int eos_id) {
    auto transformer = static_cast<TransformerModel*>(model_.get());
    std::vector<BeamState> beams;

    // Initialization
    {
        BeamState initial;
        initial.tokens = prompt_tokens;
        initial.score = 0.0f;

        if (!prompt_tokens.empty()) {
            Matrix prompt_matrix({1, prompt_tokens.size()});
            for (size_t i = 0; i < prompt_tokens.size(); ++i) prompt_matrix({0, i}) = static_cast<float>(prompt_tokens[i]);

            std::vector<std::pair<Matrix, Matrix>> next_cache;
            Matrix all_logits = transformer->forward_inference(prompt_matrix, {}, next_cache);
            initial.kv_cache = next_cache;

            initial.last_logits = Matrix({1, all_logits.cols()});
            for (size_t c = 0; c < all_logits.cols(); ++c) initial.last_logits({0, c}) = all_logits({all_logits.rows() - 1, c});
        } else {
            initial.last_logits = Matrix({1, (size_t)model_->get_vocab_size()});
        }
        beams.push_back(initial);
    }

    for (int i = 0; i < n_new_tokens; ++i) {
        std::vector<BeamState> candidates;
        for (const auto& beam : beams) {
            if (!beam.tokens.empty() && beam.tokens.back() == eos_id) {
                candidates.push_back(beam);
                continue;
            }

            Matrix probs = softmax(beam.last_logits);
            std::vector<std::pair<float, int>> token_probs;
            for (size_t c = 0; c < probs.cols(); ++c) token_probs.push_back({probs({0, c}), (int)c});
            std::sort(token_probs.rbegin(), token_probs.rend());

            for (int j = 0; j < beam_width && j < (int)token_probs.size(); ++j) {
                if (token_probs[j].first <= 0) continue;

                BeamState next = beam;
                next.tokens.push_back(token_probs[j].second);
                next.score += std::log(token_probs[j].first);

                if (token_probs[j].second != eos_id) {
                    Matrix input({1, 1});
                    input({0, 0}) = static_cast<float>(token_probs[j].second);
                    std::vector<std::pair<Matrix, Matrix>> next_cache;
                    next.last_logits = transformer->forward_inference(input, beam.kv_cache, next_cache);
                    next.kv_cache = next_cache;
                }
                candidates.push_back(next);
            }
        }

        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.score > b.score; });
        beams.clear();
        for (size_t j = 0; j < std::min((size_t)candidates.size(), (size_t)beam_width); ++j) beams.push_back(candidates[j]);

        if (beams.empty()) break;
        bool all_done = true;
        for (const auto& b : beams) if (b.tokens.empty() || b.tokens.back() != eos_id) { all_done = false; break; }
        if (all_done) break;
    }

    return beams[0].tokens;
}

std::vector<int> StellarGenerator::contrastive_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, float alpha, int eos_id) {
    auto transformer = static_cast<TransformerModel*>(model_.get());
    std::vector<int> sequence = prompt_tokens;
    std::vector<std::pair<Matrix, Matrix>> kv_cache;
    Matrix logits;

    // Initial pass
    if (!prompt_tokens.empty()) {
        Matrix prompt_matrix({1, prompt_tokens.size()});
        for (size_t i = 0; i < prompt_tokens.size(); ++i) prompt_matrix({0, i}) = static_cast<float>(prompt_tokens[i]);
        std::vector<std::pair<Matrix, Matrix>> next_cache;
        Matrix all_logits = transformer->forward_inference(prompt_matrix, {}, next_cache);
        kv_cache = next_cache;
        logits = Matrix({1, all_logits.cols()});
        for (size_t c = 0; c < all_logits.cols(); ++c) logits({0, c}) = all_logits({all_logits.rows() - 1, c});
    } else {
        logits = Matrix({1, (size_t)model_->get_vocab_size()});
    }

    const Matrix& all_embeddings = transformer->get_embeddings();
    std::vector<Matrix> context_embeddings;
    for (int tid : prompt_tokens) {
        Matrix emb({1, all_embeddings.cols()});
        for (size_t k = 0; k < all_embeddings.cols(); ++k) emb({0, k}) = all_embeddings({(size_t)tid, k});
        context_embeddings.push_back(emb);
    }

    for (int i = 0; i < n_new_tokens; ++i) {
        Matrix probs = softmax(logits);
        std::vector<std::pair<float, int>> top_k;
        for (size_t c = 0; c < probs.cols(); ++c) top_k.push_back({probs({0, c}), (int)c});
        std::sort(top_k.rbegin(), top_k.rend());

        int best_token = -1;
        float max_score = -std::numeric_limits<float>::infinity();

        for (int j = 0; j < beam_width && j < (int)top_k.size(); ++j) {
            int candidate_id = top_k[j].second;
            float confidence = top_k[j].first;

            Matrix cand_emb({1, all_embeddings.cols()});
            for (size_t k = 0; k < all_embeddings.cols(); ++k) cand_emb({0, k}) = all_embeddings({(size_t)candidate_id, k});

            float max_sim = -1.0f;
            for (const auto& ctx : context_embeddings) {
                float dot = 0, n1 = 0, n2 = 0;
                for (size_t k = 0; k < all_embeddings.cols(); ++k) {
                    dot += ctx({0, k}) * cand_emb({0, k});
                    n1 += ctx({0, k}) * ctx({0, k});
                    n2 += cand_emb({0, k}) * cand_emb({0, k});
                }
                float sim = dot / (std::sqrt(n1) * std::sqrt(n2) + 1e-9f);
                if (sim > max_sim) max_sim = sim;
            }

            float score = (1.0f - alpha) * confidence - alpha * max_sim;
            if (score > max_score) { max_score = score; best_token = candidate_id; }
        }

        if (best_token == -1 || best_token == eos_id) break;
        sequence.push_back(best_token);

        Matrix next_emb({1, all_embeddings.cols()});
        for (size_t k = 0; k < all_embeddings.cols(); ++k) next_emb({0, k}) = all_embeddings({(size_t)best_token, k});
        context_embeddings.push_back(next_emb);

        Matrix input({1, 1});
        input({0, 0}) = static_cast<float>(best_token);
        std::vector<std::pair<Matrix, Matrix>> next_cache;
        logits = transformer->forward_inference(input, kv_cache, next_cache);
        kv_cache = next_cache;
    }

    return sequence;
}

std::vector<int> StellarGenerator::mirostat_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens, float tau, float eta, int eos_id) {
    auto transformer = static_cast<TransformerModel*>(model_.get());
    std::vector<int> sequence = prompt_tokens;
    std::vector<std::pair<Matrix, Matrix>> kv_cache;
    float max_surprise = 2.0f * tau;
    Matrix logits;

    // Initial pass
    if (!prompt_tokens.empty()) {
        Matrix prompt_matrix({1, prompt_tokens.size()});
        for (size_t i = 0; i < prompt_tokens.size(); ++i) prompt_matrix({0, i}) = static_cast<float>(prompt_tokens[i]);
        std::vector<std::pair<Matrix, Matrix>> next_cache;
        Matrix all_logits = transformer->forward_inference(prompt_matrix, {}, next_cache);
        kv_cache = next_cache;
        logits = Matrix({1, all_logits.cols()});
        for (size_t c = 0; c < all_logits.cols(); ++c) logits({0, c}) = all_logits({all_logits.rows() - 1, c});
    } else {
        logits = Matrix({1, (size_t)model_->get_vocab_size()});
    }

    for (int i = 0; i < n_new_tokens; ++i) {
        Matrix probs = softmax(logits);
        std::vector<std::pair<float, int>> token_probs;
        for (size_t c = 0; c < probs.cols(); ++c) token_probs.push_back({probs({0, c}), (int)c});
        std::sort(token_probs.rbegin(), token_probs.rend());

        int k = 0;
        for (; k < (int)token_probs.size(); ++k) {
            if (-std::log2(token_probs[k].first + 1e-9f) > max_surprise) break;
        }
        if (k == 0) k = 1;

        std::vector<float> filtered_probs;
        float sum = 0;
        for (int j = 0; j < k; ++j) { filtered_probs.push_back(token_probs[j].first); sum += token_probs[j].first; }
        for (auto& p : filtered_probs) p /= (sum + 1e-9f);

        std::discrete_distribution<> dist(filtered_probs.begin(), filtered_probs.end());
        int sampled_idx = dist(gen_);
        int token = token_probs[sampled_idx].second;

        if (token == eos_id) break;
        sequence.push_back(token);

        float surprise = -std::log2(probs({0, (size_t)token}) + 1e-9f);
        max_surprise -= eta * (surprise - tau);

        Matrix input({1, 1});
        input({0, 0}) = static_cast<float>(token);
        std::vector<std::pair<Matrix, Matrix>> next_cache;
        logits = transformer->forward_inference(input, kv_cache, next_cache);
        kv_cache = next_cache;
    }

    return sequence;
}

std::vector<int> StellarGenerator::speculative_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens) {
    if (!draft_model_) return generate(prompt_tokens, n_new_tokens);

    auto main_model = static_cast<TransformerModel*>(model_.get());
    auto draft_model = static_cast<TransformerModel*>(draft_model_.get());
    std::vector<int> sequence = prompt_tokens;
    std::vector<std::pair<Matrix, Matrix>> main_cache, draft_cache;

    if (!prompt_tokens.empty()) {
        Matrix prompt_matrix({1, prompt_tokens.size()});
        for (size_t i = 0; i < prompt_tokens.size(); ++i) prompt_matrix({0, i}) = static_cast<float>(prompt_tokens[i]);
        std::vector<std::pair<Matrix, Matrix>> nm, nd;
        main_model->forward_inference(prompt_matrix, {}, nm); main_cache = nm;
        draft_model->forward_inference(prompt_matrix, {}, nd); draft_cache = nd;
    }

    int K = 5;
    for (int i = 0; i < n_new_tokens; ) {
        std::vector<int> drafts;
        std::vector<std::pair<Matrix, Matrix>> temp_draft_cache = draft_cache;
        for (int k = 0; k < K; ++k) {
            Matrix in({1, 1}); in({0, 0}) = static_cast<float>(k == 0 ? sequence.back() : drafts.back());
            std::vector<std::pair<Matrix, Matrix>> next;
            Matrix logits = draft_model->forward_inference(in, temp_draft_cache, next);
            temp_draft_cache = next;
            drafts.push_back(sample_token(logits, sequence, i + k));
        }

        Matrix verif_in({1, drafts.size()});
        for (size_t k = 0; k < drafts.size(); ++k) verif_in({0, k}) = static_cast<float>(drafts[k]);
        std::vector<std::pair<Matrix, Matrix>> next_main;
        Matrix main_logits = main_model->forward_inference(verif_in, main_cache, next_main);

        int accepted = 0;
        bool rejected = false;
        for (int k = 0; k < K; ++k) {
            Matrix step_logits({1, main_logits.cols()});
            for (size_t c = 0; c < main_logits.cols(); ++c) step_logits({0, c}) = main_logits({(size_t)k, c});
            int main_token = sample_token(step_logits, sequence, i + k);
            if (drafts[k] == main_token) {
                sequence.push_back(drafts[k]);
                accepted++;
            } else {
                sequence.push_back(main_token);
                accepted++;
                rejected = true;
                break;
            }
        }

        Matrix sync_in({1, (size_t)accepted});
        for (int k = 0; k < accepted; ++k) sync_in({0, (size_t)k}) = static_cast<float>(sequence[sequence.size() - accepted + k]);
        std::vector<std::pair<Matrix, Matrix>> sm, sd;
        main_model->forward_inference(sync_in, main_cache, sm); main_cache = sm;
        draft_model->forward_inference(sync_in, draft_cache, sd); draft_cache = sd;

        i += accepted;
        if (rejected) continue;
    }

    return sequence;
}

int StellarGenerator::sample_token(const Matrix& logits, const std::vector<int>& past, int step) {
    Matrix l = logits;
    for (const auto& b : config_.logit_bias) if (b.first < l.cols()) l({0, (size_t)b.first}) += b.second;

    float temp = config_.temperature;
    if (!config_.temperature_schedule.empty()) temp = config_.temperature_schedule[std::min((size_t)step, config_.temperature_schedule.size() - 1)];

    if (temp == 0.0f || config_.method == "greedy") {
        int best = -1; float max_l = -std::numeric_limits<float>::infinity();
        for (size_t c = 0; c < l.cols(); ++c) if (l({0, c}) > max_l) { max_l = l({0, c}); best = (int)c; }
        return best;
    }

    l = l / temp;
    if (config_.repetition_penalty != 1.0f) {
        for (int tid : past) if (tid < (int)l.cols()) {
            if (l({0, (size_t)tid}) > 0) l({0, (size_t)tid}) /= config_.repetition_penalty;
            else l({0, (size_t)tid}) *= config_.repetition_penalty;
        }
    }

    Matrix probs = softmax(l);
    std::vector<float> pvec;
    for (size_t c = 0; c < probs.cols(); ++c) pvec.push_back(probs({0, c}));
    std::discrete_distribution<> dist(pvec.begin(), pvec.end());
    return dist(gen_);
}

} // namespace Stellar
} // namespace TissLM

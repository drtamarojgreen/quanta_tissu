#include "generator.h"
#include "core/transformer_model.h" // For static_cast to TransformerModel
#include <random>
#include <algorithm>
#include <limits>

namespace TissDB {
namespace TissLM {
namespace Core {

using namespace TissNum;

namespace {
    // Softmax function
    TissNum::Matrix softmax(const TissNum::Matrix& input) {
        TissNum::Matrix output(input.rows(), input.cols());
        for (int r = 0; r < input.rows(); ++r) {
            float max_val = -std::numeric_limits<float>::infinity();
            for (int c = 0; c < input.cols(); ++c) {
                if (input(r, c) > max_val) {
                    max_val = input(r, c);
                }
            }

            float sum_exp = 0.0f;
            for (int c = 0; c < input.cols(); ++c) {
                output(r, c) = std::exp(input(r, c) - max_val);
                sum_exp += output(r, c);
            }

            for (int c = 0; c < input.cols(); ++c) {
                output(r, c) = output(r, c) / sum_exp;
            }
        }
        return output;
    }
} // anonymous namespace

Generator::Generator(
    std::shared_ptr<Model> model,
    const Generation::GenerationConfig& config
) : model_(model), config_(config) {
}

std::vector<int> Generator::generate(const std::vector<int>& prompt_tokens, int max_new_tokens) {
    std::vector<int> generated_sequence = prompt_tokens;
    std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> kv_cache;

    auto transformer_model = static_cast<TransformerModel*>(model_.get());

    // Use forward_inference to process the prompt and build the initial KV cache
    if (!prompt_tokens.empty()) {
        TissNum::Matrix prompt_matrix(1, prompt_tokens.size());
        for (size_t i = 0; i < prompt_tokens.size(); ++i) {
            prompt_matrix(0, i) = static_cast<float>(prompt_tokens[i]);
        }
        std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> new_kv_cache;
        // We don't need the logits for the prompt, just the cache
        transformer_model->forward_inference(prompt_matrix, kv_cache, new_kv_cache);
        kv_cache = new_kv_cache;
    }

    int current_token = -1;
    if (!generated_sequence.empty()) {
        current_token = generated_sequence.back();
    }

    for (int i = 0; i < max_new_tokens; ++i) {
        TissNum::Matrix input_token(1, 1);
        input_token(0, 0) = static_cast<float>(current_token);

        std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> new_kv_cache;
        TissNum::Matrix logits = transformer_model->forward_inference(input_token, kv_cache, new_kv_cache);
        kv_cache = new_kv_cache;

        // Get the logits for the last token
        TissNum::Matrix last_token_logits(1, logits.cols());
        for(size_t c = 0; c < logits.cols(); ++c) {
            last_token_logits(0, c) = logits(logits.rows() - 1, c);
        }

        int next_token = sample_token(last_token_logits, generated_sequence); // Pass generated_sequence

        if (config_.eos_ids.size() > 0 && std::find(config_.eos_ids.begin(), config_.eos_ids.end(), next_token) != config_.eos_ids.end()) {
            break; // End of sequence token found
        }

        generated_sequence.push_back(next_token);
        current_token = next_token;
    }

    return generated_sequence;
}

int Generator::sample_token(const TissNum::Matrix& logits, const std::vector<int>& past_tokens) {
    // Apply temperature if configured
    TissNum::Matrix processed_logits = logits; // Assuming logits is (1, vocab_size)
    if (config_.temperature > 0.0f) {
        processed_logits = processed_logits / config_.temperature;
    }

    // Apply repetition penalty
    if (config_.repetition_penalty != 1.0f) {
        for (int token_id : past_tokens) {
            if (token_id < processed_logits.cols()) {
                if (processed_logits(0, token_id) > 0) {
                    processed_logits(0, token_id) /= config_.repetition_penalty;
                } else {
                    processed_logits(0, token_id) *= config_.repetition_penalty;
                }
            }
        }
    }

    // Apply n-gram repetition penalty
    if (config_.no_repeat_ngram_size > 0 && past_tokens.size() >= config_.no_repeat_ngram_size) {
        std::vector<int> ngram_prefix(past_tokens.end() - (config_.no_repeat_ngram_size - 1), past_tokens.end());
        for (size_t i = 0; i <= past_tokens.size() - config_.no_repeat_ngram_size; ++i) {
            bool match = true;
            for (size_t j = 0; j < ngram_prefix.size(); ++j) {
                if (past_tokens[i + j] != ngram_prefix[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                int banned_token = past_tokens[i + config_.no_repeat_ngram_size - 1];
                if (banned_token < processed_logits.cols()) {
                    processed_logits(0, banned_token) = -std::numeric_limits<float>::infinity();
                }
            }
        }
    }

    // Apply logit bias
    for (const auto& pair : config_.logit_bias) {
        if (pair.first < processed_logits.cols()) {
            processed_logits(0, pair.first) += pair.second;
        }
    }

    TissNum::Matrix probabilities = softmax(processed_logits);

    if (config_.method == "random" || config_.method == "sampling") {
        std::vector<float> probs_vec;
        for (int c = 0; c < probabilities.cols(); ++c) {
            probs_vec.push_back(probabilities(0, c));
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> d(probs_vec.begin(), probs_vec.end());
        return d(gen);
    }
    
    // Collect all token probabilities and their indices
    std::vector<std::pair<float, int>> token_probs;
    for (int c = 0; c < probabilities.cols(); ++c) {
        token_probs.push_back({probabilities(0, c), c});
    }

    // Sort by probability in descending order
    std::sort(token_probs.rbegin(), token_probs.rend());

    // Nucleus (Top-p) sampling
    if (config_.method == "nucleus" && config_.top_p.has_value() && config_.top_p.value() < 1.0f) {
        float cumulative_probability = 0.0f;
        size_t last_idx = 0;
        for (size_t i = 0; i < token_probs.size(); ++i) {
            cumulative_probability += token_probs[i].first;
            last_idx = i;
            if (cumulative_probability >= config_.top_p.value()) {
                break;
            }
        }
        token_probs.resize(last_idx + 1);

        // Re-normalize probabilities
        float sum_top_p_probs = 0.0f;
        for (const auto& p : token_probs) {
            sum_top_p_probs += p.first;
        }
        for (auto& p : token_probs) {
            p.first /= sum_top_p_probs;
        }
    }

    // Top-k sampling (applied after Top-p if both are active)
    if (config_.method == "top_k" && config_.top_k.has_value() && config_.top_k.value() > 0 && config_.top_k.value() < token_probs.size()) {
        token_probs.resize(config_.top_k.value());

        // Re-normalize probabilities
        float sum_top_k_probs = 0.0f;
        for (const auto& p : token_probs) {
            sum_top_k_probs += p.first;
        }
        for (auto& p : token_probs) {
            p.first /= sum_top_k_probs;
        }
    }

    // Greedy sampling or final sampling from filtered/normalized probabilities
    if (token_probs.empty()) {
        return -1; // Should not happen with valid logits
    }

    if (config_.method == "greedy") {
        return token_probs[0].second;
    } else {
        // Sample from the (potentially filtered and normalized) probabilities
        std::vector<float> normalized_probs;
        for (const auto& p : token_probs) {
            normalized_probs.push_back(p.first);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> d(normalized_probs.begin(), normalized_probs.end());

        int sampled_index = d(gen);
        return token_probs[sampled_index].second;
    }
}

std::vector<int> Generator::beam_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, int eos_id) {
    std::vector<std::pair<std::vector<int>, float>> beams;
    beams.push_back({prompt_tokens, 0.0f});

    for (int i = 0; i < n_new_tokens; ++i) {
        std::vector<std::pair<std::vector<int>, float>> new_beams;
        for (const auto& beam : beams) {
            const std::vector<int>& seq = beam.first;
            float score = beam.second;

            if (seq.back() == eos_id) {
                new_beams.push_back(beam);
                continue;
            }

            // NOTE: This is inefficient as it re-processes the whole sequence every time.
            // A proper implementation would use a KV cache for each beam.
            TissNum::Matrix prompt_matrix(1, seq.size());
            for (size_t j = 0; j < seq.size(); ++j) {
                prompt_matrix(0, j) = static_cast<float>(seq[j]);
            }
            TissNum::Matrix logits = model_->forward(prompt_matrix);
            TissNum::Matrix last_token_logits(1, logits.cols());
            for(size_t c = 0; c < logits.cols(); ++c) {
                last_token_logits(0, c) = logits(logits.rows() - 1, c);
            }

            TissNum::Matrix probabilities = softmax(last_token_logits);

            std::vector<std::pair<float, int>> token_probs;
            for (int c = 0; c < probabilities.cols(); ++c) {
                token_probs.push_back({probabilities(0, c), c});
            }
            std::sort(token_probs.rbegin(), token_probs.rend());

            for (int j = 0; j < beam_width && j < token_probs.size(); ++j) {
                int next_token = token_probs[j].second;
                float new_score = score + std::log(token_probs[j].first);
                std::vector<int> new_seq = seq;
                new_seq.push_back(next_token);
                new_beams.push_back({new_seq, new_score});
            }
        }

        std::sort(new_beams.begin(), new_beams.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        beams.assign(new_beams.begin(), new_beams.begin() + std::min((int)new_beams.size(), beam_width));
    }

    return beams[0].first;
}

std::vector<int> Generator::contrastive_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, float alpha, int eos_id) {
    std::vector<int> current_tokens = prompt_tokens;
    std::vector<int> generated_tokens;

    auto transformer_model = static_cast<TransformerModel*>(model_.get());

    for (int i = 0; i < n_new_tokens; ++i) {
        TissNum::Matrix prompt_matrix(1, current_tokens.size());
        for (size_t j = 0; j < current_tokens.size(); ++j) {
            prompt_matrix(0, j) = static_cast<float>(current_tokens[j]);
        }
        TissNum::Matrix logits = model_->forward(prompt_matrix);
        TissNum::Matrix last_token_logits(1, logits.cols());
        for(size_t c = 0; c < logits.cols(); ++c) {
            last_token_logits(0, c) = logits(logits.rows() - 1, c);
        }

        TissNum::Matrix probabilities = softmax(last_token_logits);

        std::vector<std::pair<float, int>> token_probs;
        for (int c = 0; c < probabilities.cols(); ++c) {
            token_probs.push_back({probabilities(0, c), c});
        }
        std::sort(token_probs.rbegin(), token_probs.rend());

        std::vector<int> top_k_indices;
        for (int j = 0; j < beam_width && j < token_probs.size(); ++j) {
            top_k_indices.push_back(token_probs[j].second);
        }

        const TissNum::Matrix& embeddings = transformer_model->get_embeddings();

        TissNum::Matrix context_embeddings(current_tokens.size(), embeddings.cols());
        for (size_t j = 0; j < current_tokens.size(); ++j) {
            for (int k = 0; k < embeddings.cols(); ++k) {
                context_embeddings(j, k) = embeddings(current_tokens[j], k);
            }
        }

        int best_token = -1;
        float max_score = -std::numeric_limits<float>::infinity();

        for (int token_id : top_k_indices) {
            float model_confidence = probabilities(0, token_id);

            TissNum::Matrix candidate_embedding(1, embeddings.cols());
            for (int k = 0; k < embeddings.cols(); ++k) {
                candidate_embedding(0, k) = embeddings(token_id, k);
            }

            float max_sim = -1.0f;
            for (int j = 0; j < context_embeddings.rows(); ++j) {
                float dot = 0.0f;
                float norm_a = 0.0f;
                float norm_b = 0.0f;
                for (int k = 0; k < context_embeddings.cols(); ++k) {
                    dot += context_embeddings(j, k) * candidate_embedding(0, k);
                    norm_a += context_embeddings(j, k) * context_embeddings(j, k);
                    norm_b += candidate_embedding(0, k) * candidate_embedding(0, k);
                }
                float sim = dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
                if (sim > max_sim) {
                    max_sim = sim;
                }
            }
            float degeneration_penalty = max_sim;

            float score = (1 - alpha) * model_confidence - alpha * degeneration_penalty;

            if (score > max_score) {
                max_score = score;
                best_token = token_id;
            }
        }

        if (best_token == eos_id) {
            break;
        }

        generated_tokens.push_back(best_token);
        current_tokens.push_back(best_token);
    }

    return generated_tokens;
}

std::vector<int> Generator::mirostat_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens, float tau, float eta, int eos_id) {
    std::vector<int> current_tokens = prompt_tokens;
    std::vector<int> generated_tokens;
    float max_surprise = 2 * tau;

    for (int i = 0; i < n_new_tokens; ++i) {
        TissNum::Matrix prompt_matrix(1, current_tokens.size());
        for (size_t j = 0; j < current_tokens.size(); ++j) {
            prompt_matrix(0, j) = static_cast<float>(current_tokens[j]);
        }
        TissNum::Matrix logits = model_->forward(prompt_matrix);
        TissNum::Matrix last_token_logits(1, logits.cols());
        for(size_t c = 0; c < logits.cols(); ++c) {
            last_token_logits(0, c) = logits(logits.rows() - 1, c);
        }

        TissNum::Matrix probabilities = softmax(last_token_logits);

        std::vector<std::pair<float, int>> token_probs;
        for (int c = 0; c < probabilities.cols(); ++c) {
            token_probs.push_back({probabilities(0, c), c});
        }
        std::sort(token_probs.rbegin(), token_probs.rend());

        std::vector<float> surprises;
        for (const auto& p : token_probs) {
            surprises.push_back(-std::log2(p.first));
        }

        int k = 0;
        for (k = 0; k < surprises.size(); ++k) {
            if (surprises[k] > max_surprise) {
                break;
            }
        }
        if (k == 0) {
            k = surprises.size();
        }

        std::vector<int> nucleus_indices;
        float sum_nucleus_probs = 0.0f;
        for (int j = 0; j < k; ++j) {
            nucleus_indices.push_back(token_probs[j].second);
            sum_nucleus_probs += token_probs[j].first;
        }

        std::vector<float> nucleus_probs;
        for (int j = 0; j < k; ++j) {
            nucleus_probs.push_back(token_probs[j].first / sum_nucleus_probs);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> d(nucleus_probs.begin(), nucleus_probs.end());
        int sampled_index = d(gen);
        int next_token = nucleus_indices[sampled_index];

        float observed_surprise = -std::log2(probabilities(0, next_token));
        max_surprise -= eta * (observed_surprise - tau);

        if (next_token == eos_id) {
            break;
        }

        generated_tokens.push_back(next_token);
    }
    return generated_tokens;
}

std::vector<int> Generator::speculative_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens) {
    // TODO: Implement speculative sampling with a draft model.
    // For now, defaulting to greedy sampling.
    Generation::GenerationConfig greedy_config = Generation::GenerationConfig::greedy();
    Generator greedy_generator(model_, greedy_config);
    return greedy_generator.generate(prompt_tokens, n_new_tokens);
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

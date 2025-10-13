#include "generator.h"
#include "core/transformer_model.h" // For static_cast to TransformerModel
#include <random>
#include <algorithm>
#include <limits>

namespace TissDB {
namespace TissLM {
namespace Core {

using namespace TissNum;

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

        int next_token = sample_token(last_token_logits);

        if (config_.eos_ids.size() > 0 && std::find(config_.eos_ids.begin(), config_.eos_ids.end(), next_token) != config_.eos_ids.end()) {
            break; // End of sequence token found
        }

        generated_sequence.push_back(next_token);
        current_token = next_token;
    }

    return generated_sequence;
}

int Generator::sample_token(const TissNum::Matrix& logits) {
    // Apply temperature if configured
    TissNum::Matrix processed_logits = logits; // Assuming logits is (1, vocab_size)
    if (config_.temperature > 0.0f) {
        processed_logits = processed_logits / config_.temperature;
    }

    // Softmax function (re-implemented here for clarity, could be a shared utility)
    auto softmax = [](const TissNum::Matrix& input) -> TissNum::Matrix {
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
    };

    TissNum::Matrix probabilities = softmax(processed_logits);

    // Collect all token probabilities and their indices
    std::vector<std::pair<float, int>> token_probs;
    for (int c = 0; c < probabilities.cols(); ++c) {
        token_probs.push_back({probabilities(0, c), c});
    }

    // Sort by probability in descending order
    std::sort(token_probs.rbegin(), token_probs.rend());

    // Nucleus (Top-p) sampling
    if (config_.top_p.has_value() && config_.top_p.value() < 1.0f) {
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
    if (config_.top_k.has_value() && config_.top_k.value() > 0 && config_.top_k.value() < token_probs.size()) {
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

    if (config_.top_k.value_or(0) <= 1 && (!config_.top_p.has_value() || config_.top_p.value() >= 1.0f)) {
        // Greedy sampling (or if top_k is 1, it's effectively greedy)
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

} // namespace Core
} // namespace TissLM
} // namespace TissDB

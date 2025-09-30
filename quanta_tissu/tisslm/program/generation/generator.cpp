#include "generator.h"
#include <random>
#include <algorithm>
#include <limits>

namespace TissDB {
namespace TissLM {
namespace Core {

Generator::Generator(
    std::shared_ptr<Model> model,
    const GenerationConfig& config
) : model_(model), config_(config) {
}

int Generator::sample_token(const Matrix& logits) {
    // Apply temperature if configured
    Matrix processed_logits = logits; // Assuming logits is (1, vocab_size)
    if (config_.temperature > 0.0f) {
        processed_logits = processed_logits / config_.temperature;
    }

    // Softmax function (re-implemented here for clarity, could be a shared utility)
    auto softmax = [](const Matrix& input) -> Matrix {
        Matrix output(input.rows(), input.cols());
        for (int r = 0; r < input.rows(); ++r) {
            float max_val = -std::numeric_limits<float>::infinity();
            for (int c = 0; c < input.cols(); ++c) {
                if (input.get(r, c) > max_val) {
                    max_val = input.get(r, c);
                }
            }

            float sum_exp = 0.0f;
            for (int c = 0; c < input.cols(); ++c) {
                output.set(r, c, std::exp(input.get(r, c) - max_val));
                sum_exp += output.get(r, c);
            }

            for (int c = 0; c < input.cols(); ++c) {
                output.set(r, c, output.get(r, c) / sum_exp);
            }
        }
        return output;
    };

    Matrix probabilities = softmax(processed_logits);

    // Collect all token probabilities and their indices
    std::vector<std::pair<float, int>> token_probs;
    for (int c = 0; c < probabilities.cols(); ++c) {
        token_probs.push_back({probabilities.get(0, c), c});
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

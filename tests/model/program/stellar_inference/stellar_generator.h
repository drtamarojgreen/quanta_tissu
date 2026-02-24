#ifndef TISSLM_STELLAR_GENERATOR_H
#define TISSLM_STELLAR_GENERATOR_H

/**
 * @file stellar_generator.h
 * @brief High-performance, modular generation engine for the TissLM ecosystem.
 *
 * This module implements the "Stellar" vision of text generation, prioritizing
 * efficiency through advanced KV-cache management, speculative decoding,
 * and robust sampling strategies.
 */

#include "core/model_interface.h"
#include "generation/generation_config.h"
#include <memory>
#include <vector>
#include <string>
#include <random>
#include <map>

namespace TissLM {
namespace Stellar {

/**
 * @class StellarGenerator
 * @brief An advanced text generation engine supporting multiple optimized strategies.
 */
class StellarGenerator {
public:
    /**
     * @brief Constructs a StellarGenerator with a main model.
     * @param model Shared pointer to the primary transformer model.
     * @param config Configuration for generation parameters.
     */
    StellarGenerator(
        std::shared_ptr<TissLM::Core::Model> model,
        const TissLM::Generation::GenerationConfig& config
    );

    /**
     * @brief Constructs a StellarGenerator with a main model and a draft model for speculative sampling.
     * @param model Shared pointer to the primary transformer model.
     * @param draft_model Shared pointer to a smaller, faster model for token speculation.
     * @param config Configuration for generation parameters.
     */
    StellarGenerator(
        std::shared_ptr<TissLM::Core::Model> model,
        std::shared_ptr<TissLM::Core::Model> draft_model,
        const TissLM::Generation::GenerationConfig& config
    );

    /**
     * @brief Generates a sequence of tokens from a prompt.
     * @param prompt_tokens The initial sequence of token IDs.
     * @param max_new_tokens The maximum number of tokens to generate.
     * @return A vector of token IDs including the prompt and the generated tokens.
     */
    std::vector<int> generate(const std::vector<int>& prompt_tokens, int max_new_tokens);

    /**
     * @brief Optimized Beam Search implementation with KV-cache for each beam.
     */
    std::vector<int> beam_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, int eos_id);

    /**
     * @brief Optimized Contrastive Search implementation with KV-cache.
     */
    std::vector<int> contrastive_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, float alpha, int eos_id);

    /**
     * @brief Optimized Mirostat Sampling implementation with KV-cache.
     */
    std::vector<int> mirostat_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens, float tau, float eta, int eos_id);

    /**
     * @brief Advanced Speculative Sampling implementation.
     */
    std::vector<int> speculative_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens);

private:
    /**
     * @brief Internal helper for sampling a single token from logits.
     */
    int sample_token(const TissNum::Matrix& logits, const std::vector<int>& past_tokens, int current_step);

    std::shared_ptr<TissLM::Core::Model> model_;
    std::shared_ptr<TissLM::Core::Model> draft_model_;
    TissLM::Generation::GenerationConfig config_;
    std::mt19937 gen_;

    // Internal state for KV-cache optimization in methods like beam search
    struct BeamState {
        std::vector<int> tokens;
        float score;
        std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>> kv_cache;
        TissNum::Matrix last_logits;
    };
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_GENERATOR_H

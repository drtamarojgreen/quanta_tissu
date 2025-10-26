#ifndef TISSLM_GENERATOR_H
#define TISSLM_GENERATOR_H

#include "core/model_interface.h"
#include "generation_config.h"
#include <memory>
#include <vector>
#include <string>

namespace TissLM {
namespace Generation {

class Generator {
public:
    Generator(
        std::shared_ptr<TissLM::Core::Model> model,
        const Generation::GenerationConfig& config
    );

    Generator(
        std::shared_ptr<TissLM::Core::Model> model,
        std::shared_ptr<TissLM::Core::Model> draft_model,
        const Generation::GenerationConfig& config
    );

    std::vector<int> generate(const std::vector<int>& prompt_tokens, int max_new_tokens);
    std::vector<std::vector<int>> generate_batch(const std::vector<std::vector<int>>& prompts, int max_new_tokens);

    // Helper for sampling
    int sample_token(const TissNum::Matrix& logits, const std::vector<int>& past_tokens, int current_step);

    std::vector<int> beam_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, int eos_id);

    std::vector<int> contrastive_search(const std::vector<int>& prompt_tokens, int n_new_tokens, int beam_width, float alpha, int eos_id);

    std::vector<int> mirostat_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens, float tau, float eta, int eos_id);

    std::vector<int> speculative_sampling(const std::vector<int>& prompt_tokens, int n_new_tokens);

private:
    std::shared_ptr<TissLM::Core::Model> model_;
    std::shared_ptr<TissLM::Core::Model> draft_model_;
    Generation::GenerationConfig config_;
};

} // namespace Generation
} // namespace TissLM

#endif // TISSLM_GENERATOR_H

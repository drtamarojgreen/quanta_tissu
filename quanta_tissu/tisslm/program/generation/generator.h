#ifndef TISSLM_GENERATOR_H
#define TISSLM_GENERATOR_H

#include "core/model_interface.h"
#include "generation_config.h"
#include <memory>
#include <vector>
#include <string>

namespace TissDB {
namespace TissLM {
namespace Core {

class Generator {
public:
    Generator(
        std::shared_ptr<Model> model,
        const Generation::GenerationConfig& config
    );

    std::vector<int> generate(const std::vector<int>& prompt_tokens, int max_new_tokens);

    // Helper for sampling
    int sample_token(const TissNum::Matrix& logits);

private:
    std::shared_ptr<Model> model_;
    Generation::GenerationConfig config_;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_GENERATOR_H

#ifndef TISSLM_GENERATOR_H
#define TISSLM_GENERATOR_H

#include "../../core/model_interface.h"
#include "generation_config.h"
#nclude <memory>
#include <vector>
#include <string>

namespace TissDB {
namespace TissLM {
namespace Core {

class Generator {
public:
    Generator(
        std::shared_ptr<Model> model,
        const GenerationConfig& config
    );

    std::vector<int> generate(const std::vector<int>& prompt_tokens, int max_new_tokens);

private:
    std::shared_ptr<Model> model_;
    GenerationConfig config_;

    // Helper for sampling
    int sample_token(const Matrix& logits);
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_GENERATOR_H

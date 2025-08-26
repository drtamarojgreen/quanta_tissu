#ifndef CLLM_MODEL_H
#define CLLM_MODEL_H

#include "config.h"
#include "transformer_block.h"
#include <vector>
#include <memory>

namespace cllm {

class Model {
public:
    explicit Model(const ModelConfig& config);

    // Placeholder for the forward pass
    void forward();

private:
    ModelConfig config_;
    std::vector<std::unique_ptr<TransformerBlock>> layers_;
};

} // namespace cllm

#endif // CLLM_MODEL_H

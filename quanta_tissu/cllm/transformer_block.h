#ifndef CLLM_TRANSFORMER_BLOCK_H
#define CLLM_TRANSFORMER_BLOCK_H

#include "config.h"
#include <memory>

namespace cllm {

// Forward declarations for components
class MultiHeadAttention;
class FeedForward;

class TransformerBlock {
public:
    explicit TransformerBlock(const ModelConfig& config);

    // Placeholder for the forward pass
    void forward();

private:
    ModelConfig config_;
    std::unique_ptr<MultiHeadAttention> attention_;
    std::unique_ptr<FeedForward> ffn_;

    // Layer normalization would also be here
};

} // namespace cllm

#endif // CLLM_TRANSFORMER_BLOCK_H

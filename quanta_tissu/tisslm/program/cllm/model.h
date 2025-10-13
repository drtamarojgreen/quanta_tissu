#ifndef CLLM_MODEL_H
#define CLLM_MODEL_H

#include "config.h"
#include "transformer_block.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace cllm {

class Model {
public:
    explicit Model(const ModelConfig& config);

    // Performs the full forward pass for the model.
    // Takes a vector of token IDs and returns logits over the vocabulary.
    Eigen::MatrixXf forward(const std::vector<int>& input_tokens);

private:
    // Helper for positional encoding
    void add_positional_encoding(Eigen::MatrixXf& input);

    ModelConfig config_;

    // Token embedding lookup table
    Eigen::MatrixXf token_embeddings_;

    // Transformer blocks
    std::vector<std::unique_ptr<TransformerBlock>> layers_;

    // Final linear layer to project to vocab size
    Eigen::MatrixXf output_layer_weight_;
};

} // namespace cllm

#endif // CLLM_MODEL_H

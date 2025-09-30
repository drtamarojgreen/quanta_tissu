#ifndef TISSLM_TRANSFORMER_MODEL_H
#define TISSLM_TRANSFORMER_MODEL_H

#include "model_interface.h"
#include "embedding.h"
#include "positionalencoding.h"
#include "transformerblock.h"
#include "parameter.h" // For output layer parameters

#include <vector>
#include <memory>

namespace TissDB {
namespace TissLM {
namespace Core {

class TransformerModel : public Model {
public:
    TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, float dropout_rate, int lora_rank = 0);

    Matrix forward(const Matrix& input_tokens) override; // input_tokens would be a batch of token IDs
    Matrix forward_inference(const Matrix& input_tokens, const std::vector<std::pair<Matrix, Matrix>>& past_kv_cache, std::vector<std::pair<Matrix, Matrix>>& new_kv_cache);
    Matrix backward(const Matrix& grad_output) override;
    std::vector<std::shared_ptr<Parameter>> get_parameters() override;

private:
    Embedding embedding_layer_;
    PositionalEncoding positional_encoding_layer_;
    std::vector<TransformerBlock> transformer_blocks_;
    
    // Output linear layer
    Parameter output_weight_;
    Parameter output_bias_;

    int vocab_size_;
    int embed_dim_;
    int num_layers_;

    // Store intermediate activations for backward pass
    Matrix embedded_input_;
    std::vector<Matrix> transformer_block_outputs_;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_TRANSFORMER_MODEL_H

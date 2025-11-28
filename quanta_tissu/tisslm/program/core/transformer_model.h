#ifndef TISSLM_TRANSFORMER_MODEL_H
#define TISSLM_TRANSFORMER_MODEL_H

#include "model_interface.h"
#include "embedding.h"
#include "positionalencoding.h"
#include "transformerblock.h"
#include "parameter.h" // For output layer parameters
#include <optional>

#include <vector>
#include <memory>

namespace TissLM {
namespace Core {

class TransformerModel : public Model {
public:
    TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, int d_ff, float dropout_rate, int lora_rank = 0);

    TissNum::Matrix forward(const TissNum::Matrix& input_tokens, bool training = true) override; // input_tokens would be a batch of token IDs
    TissNum::Matrix forward_inference(const TissNum::Matrix& input_tokens, const std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>>& past_kv_cache, std::vector<std::pair<TissNum::Matrix, TissNum::Matrix>>& new_kv_cache);
    void backward(const TissNum::Matrix& grad_output) override;
    std::vector<std::shared_ptr<TissNum::Parameter>> get_parameters() override;
    std::vector<std::vector<float>> get_embeddings_as_vectors() const override;

    int get_vocab_size() const override { return vocab_size_; }

    const TissNum::Matrix& get_embeddings() const;

private:
    TissNum::Embedding embedding_layer_;
    TissNum::PositionalEncoding positional_encoding_layer_;
    std::vector<TissNum::TransformerBlock> transformer_blocks_;
    TissNum::LayerNorm final_layer_norm_;
    
    // Output linear layer
    TissNum::Parameter output_weight_;
    TissNum::Parameter output_bias_;

    int vocab_size_;
    int embed_dim_;
    int num_layers_;

    // Store intermediate activations for backward pass
    TissNum::Matrix embedded_input_;
    std::vector<TissNum::Matrix> transformer_block_outputs_;
    TissNum::Matrix final_layer_norm_output_; // Store output of final layer norm
    std::vector<size_t> cached_token_ids_; // Store token IDs for backward pass
};

} // namespace Core
} // namespace TissLM

#endif // TISSLM_TRANSFORMER_MODEL_H

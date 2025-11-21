#include "transformerblock.h"
#include <iostream>

namespace TissNum {

// Constructor for the TransformerBlock class.
// Initializes the sub-modules of the transformer block.
TransformerBlock::TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, int lora_rank, const std::string& name, AttentionMode attention_mode)
    // Initializer list begins.
    // Initialize the Multi-Head Attention layer.
    : mha_(d_model, num_heads, lora_rank, name + ".mha", attention_mode),
      // Initialize the Feed-Forward Network layer.
      ffn_(d_model, d_ff, name + ".ffn"),
      // Initialize the first Layer Normalization layer.
      ln1_(d_model, name + ".ln1"),
      // Initialize the second Layer Normalization layer.
      ln2_(d_model, name + ".ln2"),
      // Initialize the first dropout layer.
      dropout1_(dropout_p),
      // Initialize the second dropout layer.
      dropout2_(dropout_p) {} // End of constructor body.

// Performs the forward pass of the TransformerBlock.
// This function takes an input matrix 'x' and processes it through the block's layers.
// It supports attention masking and KV caching for efficient inference.
Matrix TransformerBlock::forward(const Matrix& x, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache, bool training) {
    // 1. Pre-Normalization before MHA
    Matrix x_norm1 = ln1_.forward(x);
    x_norm1_ = x_norm1;

    // 2. Multi-Head Attention
    Matrix attn_out = mha_.forward(x_norm1, x_norm1, x_norm1, mask, past_kv, new_kv_cache);
    attn_out_ = attn_out;
    attn_out = dropout1_.forward(attn_out, training);

    // 3. First Residual Connection
    Matrix x_plus_attn = x + attn_out;
    x_plus_attn_ = x_plus_attn;

    // 4. Pre-Normalization before FFN
    Matrix x_norm2 = ln2_.forward(x_plus_attn);
    x_norm2_ = x_norm2;

    // 5. Feed-Forward Network
    Matrix ffn_out = ffn_.forward(x_norm2);
    ffn_out_ = ffn_out;
    ffn_out = dropout2_.forward(ffn_out, training);

    // 6. Second Residual Connection
    Matrix x_plus_ffn = x_plus_attn + ffn_out;
    x_plus_ffn_ = x_plus_ffn;

    // This block caches intermediate values if the model is in training mode.
    // These cached values are needed for the backward pass (backpropagation).
    if (training) {
        // Cache the original input.
        cached_x_ = x;
        // Cache the result of the first layer normalization.
        cached_x_norm1_ = x_norm1;
        // Cache the result of the first residual connection.
        cached_x_plus_attn_ = x_plus_attn;
        // Cache the result of the second layer normalization.
        cached_x_norm2_ = x_norm2;
        // Cache the final output.
        cached_x_plus_ffn_ = x_plus_ffn;
    }

    // The final output of the transformer block's forward pass is returned.
    return x_plus_ffn;
}

// Performs the backward pass of the TransformerBlock for backpropagation.
// This function calculates the gradients with respect to the block's inputs
// based on the gradients from the subsequent layer (d_out).
Matrix TransformerBlock::backward(const Matrix& d_out) {
    // 1. Backprop through second residual
    Matrix d_x_plus_attn = d_out;
    Matrix d_ffn_out = d_out;

    // 2. Backprop through FFN
    d_ffn_out = dropout2_.backward(d_ffn_out);
    Matrix d_x_norm2 = ffn_.backward(d_ffn_out);

    // 3. Backprop through second norm
    Matrix d_x_plus_attn_from_norm = ln2_.backward(d_x_norm2);
    d_x_plus_attn += d_x_plus_attn_from_norm;

    // 4. Backprop through first residual
    Matrix d_x = d_x_plus_attn;
    Matrix d_attn_out = d_x_plus_attn;

    // 5. Backprop through MHA
    d_attn_out = dropout1_.backward(d_attn_out);
    Matrix d_x_norm1 = mha_.backward(d_attn_out);

    // 6. Backprop through first norm
    Matrix d_x_from_norm = ln1_.backward(d_x_norm1);
    d_x += d_x_from_norm;

    return d_x;
}

// Retrieves all learnable parameters from the sub-modules of this block.
// This is used by the optimizer to update the model's weights.
std::vector<Parameter*> TransformerBlock::parameters() {
    // Create a vector to hold pointers to the parameters.
    std::vector<Parameter*> params;
    // Get parameters from the multi-head attention layer.
    auto mha_params = mha_.parameters();
    // Add the MHA parameters to the main vector.
    params.insert(params.end(), mha_params.begin(), mha_params.end());
    // Get parameters from the feed-forward network layer.
    auto ffn_params = ffn_.parameters();
    // Add the FFN parameters to the main vector.
    params.insert(params.end(), ffn_params.begin(), ffn_params.end());
    // Get parameters from the first layer normalization layer.
    auto ln1_params = ln1_.parameters();
    // Add the first LayerNorm parameters to the main vector.
    params.insert(params.end(), ln1_params.begin(), ln1_params.end());
    // Get parameters from the second layer normalization layer.
    auto ln2_params = ln2_.parameters();
    // Add the second LayerNorm parameters to the main vector.
    params.insert(params.end(), ln2_params.begin(), ln2_params.end());
    // Return the vector containing all parameters.
    return params;
}

} // namespace TissNum
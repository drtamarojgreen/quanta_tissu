#include "transformerblock.h"
#include <iostream>

namespace TissNum {

// Constructor for the TransformerBlock class.
// Initializes the sub-modules of the transformer block.
TransformerBlock::TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, int lora_rank, const std::string& name)
    // Initializer list begins.
    // Initialize the Multi-Head Attention layer.
    : mha_(d_model, num_heads, lora_rank, name + ".mha"),
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
    // Declare a matrix to hold the output of the first layer normalization.
    Matrix x_norm1;

    // This section handles the self-attention mechanism.
    // It is the first sub-layer of the transformer block.
    // The multi-head attention layer is called with the input 'x' for query, key, and value.
    Matrix attn_out = mha_.forward(x, x, x, mask, past_kv, new_kv_cache);
    // Assign the value of 'attn_out' to the member variable 'attn_out_'.
    attn_out_ = attn_out;
    // Dropout is applied to the output of the attention layer.
    attn_out = dropout1_.forward(attn_out, training);

    // The first residual connection (Add).
    // The original input 'x' is added to the output of the attention sub-layer.
    Matrix x_plus_attn = x + attn_out;
    // Assign the value of 'x_plus_attn' to the member variable 'x_plus_attn_'.
    x_plus_attn_ = x_plus_attn;
    // The first layer normalization (Norm).
    // The result of the residual connection is normalized.
    x_norm1 = ln1_.forward(x_plus_attn);
    // Assign the value of 'x_norm1' to the member variable 'x_norm1_'.
    x_norm1_ = x_norm1;

    // This section handles the feed-forward network.
    // It is the second sub-layer of the transformer block.
    // The feed-forward network is applied to the output of the first layer normalization.
    Matrix ffn_out = ffn_.forward(x_norm1);
    // Assign the value of 'ffn_out' to the member variable 'ffn_out_'.
    ffn_out_ = ffn_out;
    // Dropout is applied to the output of the feed-forward network.
    ffn_out = dropout2_.forward(ffn_out, training);

    // The second residual connection (Add).
    // The output of the feed-forward network is added to the output of the first residual connection.
    Matrix x_plus_ffn = x_plus_attn + ffn_out;
    // Assign the value of 'x_plus_ffn' to the member variable 'x_plus_ffn_'.
    x_plus_ffn_ = x_plus_ffn;
    // The second layer normalization (Norm).
    // The result of the second residual connection is normalized.
    Matrix x_norm2 = ln2_.forward(x_plus_ffn);
    // Assign the value of 'x_norm2' to the member variable 'x_norm2_'.
    x_norm2_ = x_norm2;

    // This block caches intermediate values if the model is in training mode.
    // These cached values are needed for the backward pass (backpropagation).
    if (training) {
        // Cache the original input.
        cached_x_ = x;
        // Cache the result of the first residual connection.
        cached_x_plus_attn_ = x_plus_attn;
        // Cache the result of the first layer normalization.
        cached_x_norm1_ = x_norm1;
        // Cache the result of the second residual connection.
        cached_x_plus_ffn_ = x_plus_ffn;
        // Cache the final normalized output.
        cached_x_norm2_ = x_norm2;
    }

    // The final output of the transformer block's forward pass is returned.
    return x_norm2;
}

// Performs the backward pass of the TransformerBlock for backpropagation.
// This function calculates the gradients with respect to the block's inputs
// based on the gradients from the subsequent layer (d_out).
Matrix TransformerBlock::backward(const Matrix& d_out) {
    // Backpropagate through the second layer normalization.
    Matrix dx_plus_ffn = ln2_.backward(d_out);

    // Backpropagate through the second residual connection.
    // The gradient is passed to both branches of the addition.
    Matrix d_x_norm1_residual = dx_plus_ffn;
    Matrix d_ffn_out = dx_plus_ffn;

    // Backpropagate through the second dropout layer.
    d_ffn_out = dropout2_.backward(d_ffn_out);

    // Backpropagate through the feed-forward network.
    Matrix d_x_norm1_ffn = ffn_.backward(d_ffn_out);

    // Combine the gradients for x_norm1 from the two paths.
    Matrix dx_norm1 = d_x_norm1_residual + d_x_norm1_ffn;

    // Backpropagate through the first layer normalization.
    Matrix dx_plus_attn = ln1_.backward(dx_norm1);

    // Backpropagate through the first residual connection.
    // The gradient is passed to both branches of the addition.
    Matrix d_x_residual = dx_plus_attn;
    Matrix d_attn_out = dx_plus_attn;

    // Backpropagate through the first dropout layer.
    d_attn_out = dropout1_.backward(d_attn_out);

    // Backpropagate through the multi-head attention layer.
    Matrix dx_mha = mha_.backward(d_attn_out);

    // Combine the gradients for the final output.
    return d_x_residual + dx_mha;
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
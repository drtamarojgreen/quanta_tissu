#include "transformer_model.h"

namespace TissDB {
namespace TissLM {
namespace Core {

TransformerModel::TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, float dropout_rate, int lora_rank)
    : embedding_layer_(vocab_size, embed_dim),
      positional_encoding_layer_(max_seq_len, embed_dim),
      output_weight_(embed_dim, vocab_size, true), // Output layer weight
      output_bias_(1, vocab_size, true), // Output layer bias
      vocab_size_(vocab_size),
      embed_dim_(embed_dim),
      num_layers_(num_layers)
{
    for (int i = 0; i < num_layers; ++i) {
        transformer_blocks_.emplace_back(embed_dim, num_heads, dropout_rate, lora_rank);
    }
}

Matrix TransformerModel::forward(const Matrix& input_tokens) {
    // 1. Embedding layer
    embedded_input_ = embedding_layer_.forward(input_tokens);

    // 2. Positional Encoding
    Matrix x = positional_encoding_layer_.forward(embedded_input_);

    // 3. Transformer Blocks
    transformer_block_outputs_.clear();
    transformer_block_outputs_.push_back(x); // Store input to first block
    for (auto& block : transformer_blocks_) {
        x = block.forward(x);
        transformer_block_outputs_.push_back(x); // Store output of each block
    }

    // 4. Output Linear Layer
    // x is (batch_size * seq_len, embed_dim)
    // output_weight_ is (embed_dim, vocab_size)
    // output_bias_ is (1, vocab_size)
    Matrix output = x.matmul(output_weight_.value) + output_bias_.value;

    return output;
}

Matrix TransformerModel::forward_inference(const Matrix& input_tokens, const std::vector<std::pair<Matrix, Matrix>>& past_kv_cache, std::vector<std::pair<Matrix, Matrix>>& new_kv_cache) {
    // Ensure new_kv_cache is correctly sized
    if (new_kv_cache.size() != num_layers_) {
        new_kv_cache.resize(num_layers_);
    }

    // 1. Embedding layer
    Matrix x = embedding_layer_.forward(input_tokens);

    // 2. Positional Encoding
    // For inference, input_tokens is typically a single token, so its position is past_kv_cache[0].first.cols()
    // Assuming input_tokens is (batch_size, 1) and we are processing one token at a time
    size_t current_position = 0;
    if (!past_kv_cache.empty() && past_kv_cache[0].first.cols() > 0) {
        current_position = past_kv_cache[0].first.cols();
    }
    x = positional_encoding_layer_.forward(x, current_position);

    // 3. Transformer Blocks
    for (int i = 0; i < num_layers_; ++i) {
        std::optional<std::pair<Matrix, Matrix>> current_past_kv = std::nullopt;
        if (i < past_kv_cache.size()) {
            current_past_kv = past_kv_cache[i];
        }
        
        std::optional<std::pair<Matrix, Matrix>> current_new_kv;
        x = transformer_blocks_[i].forward(x, Matrix(), current_past_kv, &current_new_kv);
        
        if (current_new_kv.has_value()) {
            new_kv_cache[i] = current_new_kv.value();
        }
    }

    // 4. Output Linear Layer
    Matrix output = x.matmul(output_weight_.value) + output_bias_.value;

    return output;
}

Matrix TransformerModel::backward(const Matrix& grad_output) {
    // 1. Backward through Output Linear Layer
    // grad_output is (batch_size * seq_len, vocab_size)
    // x (input to output layer) is (batch_size * seq_len, embed_dim)
    // output_weight_ is (embed_dim, vocab_size)

    // Gradient for output_bias_
    output_bias_.grad = grad_output.sum(0); // Sum along rows to get (1, vocab_size)

    // Gradient for output_weight_
    Matrix x_transpose = transformer_block_outputs_.back().transpose();
    output_weight_.grad = x_transpose.matmul(grad_output);

    // Gradient propagated back to the input of the output layer (x)
    Matrix grad_x = grad_output.matmul(output_weight_.value.transpose());

    // 2. Backward through Transformer Blocks (in reverse order)
    for (int i = num_layers_ - 1; i >= 0; --i) {
        grad_x = transformer_blocks_[i].backward(grad_x);
    }

    // 3. Backward through Positional Encoding (no gradients for positional encoding itself)
    // The gradient just passes through
    Matrix grad_embedded_input = grad_x;

    // 4. Backward through Embedding layer
    Matrix grad_input_tokens = embedding_layer_.backward(grad_embedded_input);

    return grad_input_tokens; // This would be gradients for the input token IDs, typically not used directly
}

std::vector<std::shared_ptr<Parameter>> TransformerModel::get_parameters() {
    std::vector<std::shared_ptr<Parameter>> params;

    // Embedding layer parameters
    auto embed_params = embedding_layer_.get_parameters();
    params.insert(params.end(), embed_params.begin(), embed_params.end());

    // Transformer block parameters
    for (auto& block : transformer_blocks_) {
        auto block_params = block.get_parameters();
        params.insert(params.end(), block_params.begin(), block_params.end());
    }

    // Output layer parameters
    params.push_back(std::make_shared<Parameter>(output_weight_));
    params.push_back(std::make_shared<Parameter>(output_bias_));

    return params;
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

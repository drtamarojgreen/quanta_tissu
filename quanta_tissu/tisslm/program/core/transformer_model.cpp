#include "transformer_model.h"

namespace TissDB {
namespace TissLM {
namespace Core {

using namespace TissNum;

TransformerModel::TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, float dropout_rate, int lora_rank)
    : embedding_layer_(vocab_size, embed_dim),
      positional_encoding_layer_(embed_dim, max_seq_len),
      final_layer_norm_(embed_dim),
      output_weight_(TissNum::Matrix::random(embed_dim, vocab_size), "output_weight"), // Output layer weight
      output_bias_(TissNum::Matrix::zeros(1, vocab_size), "output_bias"), // Output layer bias
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
    std::vector<size_t> token_ids(input_tokens.cols());
    for (size_t i = 0; i < input_tokens.cols(); ++i) {
        token_ids[i] = static_cast<size_t>(input_tokens(0, i));
    }
    cached_token_ids_ = token_ids; // Store for backward pass
    embedded_input_ = embedding_layer_.forward(token_ids);

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
    TissNum::Matrix output = TissNum::Matrix::matmul(x, output_weight_.value()) + output_bias_.value();

    return output;
}

Matrix TransformerModel::forward_inference(const Matrix& input_tokens, const std::vector<std::pair<Matrix, Matrix>>& past_kv_cache, std::vector<std::pair<Matrix, Matrix>>& new_kv_cache) {
    // Ensure new_kv_cache is correctly sized
    if (new_kv_cache.size() != num_layers_) {
        new_kv_cache.resize(num_layers_);
    }

    // 1. Embedding layer
    std::vector<size_t> token_ids_inference(input_tokens.cols());
    for (size_t i = 0; i < input_tokens.cols(); ++i) {
        token_ids_inference[i] = static_cast<size_t>(input_tokens(0, i));
    }
    TissNum::Matrix x = embedding_layer_.forward(token_ids_inference);

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
    TissNum::Matrix output = TissNum::Matrix::matmul(x, output_weight_.value()) + output_bias_.value();

    return output;
}

Matrix TransformerModel::backward(const Matrix& grad_output) {
    // 1. Backward through Output Linear Layer
    // grad_output is (batch_size * seq_len, vocab_size)
    // x (input to output layer) is (batch_size * seq_len, embed_dim)
    // output_weight_ is (embed_dim, vocab_size)

    // Gradient for output_bias_
    output_bias_.grad() = grad_output.sum(0); // Sum along rows to get (1, vocab_size)

    // Gradient for output_weight_
    TissNum::Matrix x_transpose = transformer_block_outputs_.back().transpose();
    output_weight_.grad() = TissNum::Matrix::matmul(x_transpose, grad_output);

    // Gradient propagated back to the input of the output layer (x)
    TissNum::Matrix grad_x = TissNum::Matrix::matmul(grad_output, output_weight_.value().transpose());

    // 2. Backward through Transformer Blocks (in reverse order)
    for (int i = num_layers_ - 1; i >= 0; --i) {
        grad_x = transformer_blocks_[i].backward(grad_x, transformer_block_outputs_[i]);
    }

    // 3. Backward through Positional Encoding (no gradients for positional encoding itself)
    // The gradient just passes through
    Matrix grad_embedded_input = grad_x;

    // 4. Backward through Embedding layer
    embedding_layer_.backward(grad_embedded_input, cached_token_ids_);

    return TissNum::Matrix(); // Gradients for input tokens are not typically propagated further
}

std::vector<std::shared_ptr<Parameter>> TransformerModel::get_parameters() {
    std::vector<std::shared_ptr<Parameter>> params;

    // Embedding layer parameters
    for (auto& p : embedding_layer_.parameters()) {
        params.push_back(std::make_shared<Parameter>(*p));
    }

    // Transformer block parameters
    for (auto& block : transformer_blocks_) {
        for (auto& p : block.parameters()) {
            params.push_back(std::make_shared<Parameter>(*p));
        }
    }

    // Output layer parameters
    params.push_back(std::make_shared<Parameter>(output_weight_));
    params.push_back(std::make_shared<Parameter>(output_bias_));

    return params;
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#include "transformer_model.h"

namespace TissLM {
namespace Core {

using namespace TissNum;

TransformerModel::TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, int d_ff, float dropout_rate, int lora_rank)
    : embedding_layer_(vocab_size, embed_dim),
      positional_encoding_layer_(embed_dim, max_seq_len),
      final_layer_norm_(embed_dim),
      output_weight_(TissNum::Matrix::random({(size_t)embed_dim, (size_t)vocab_size}), "output_weight"),
      output_bias_(TissNum::Matrix::zeros({1, (size_t)vocab_size}), "output_bias"),
      vocab_size_(vocab_size),
      embed_dim_(embed_dim),
      num_layers_(num_layers)
{
    for (int i = 0; i < num_layers; ++i) {
        transformer_blocks_.emplace_back(embed_dim, num_heads, d_ff, dropout_rate, lora_rank);
    }
}

void TransformerModel::backward(const Matrix& grad_output) {
    // 1. Backward through Output Linear Layer
    TissNum::Matrix x_transpose = final_layer_norm_output_.transpose();
    output_weight_.grad() = TissNum::Matrix::matmul(x_transpose, grad_output);
    output_bias_.grad() = grad_output.sum(0);

    // Gradient propagated back to the input of the output layer
    TissNum::Matrix grad_x = TissNum::Matrix::matmul(grad_output, output_weight_.value().transpose());

    // 2. Backward through Final Layer Norm
    grad_x = final_layer_norm_.backward(grad_x);

    // 3. Backward through Transformer Blocks (in reverse order)
    for (int i = num_layers_ - 1; i >= 0; --i) {
        grad_x = transformer_blocks_[i].backward(grad_x);
    }

    // 4. Backward through Positional Encoding (gradient passes through)
    Matrix grad_embedded_input = grad_x;

    // 5. Backward through Embedding layer
    embedding_layer_.backward(grad_embedded_input, cached_token_ids_);
}

std::vector<std::shared_ptr<TissNum::Parameter>> TransformerModel::get_parameters() {
    std::vector<std::shared_ptr<TissNum::Parameter>> params;

    auto add_params_from_raw = [&](const std::vector<TissNum::Parameter*>& raw_params) {
        for (auto* p : raw_params) {
            // No-op deleter to prevent double-freeing
            params.push_back(std::shared_ptr<TissNum::Parameter>(p, [](TissNum::Parameter*){}));
        }
    };

    add_params_from_raw(embedding_layer_.parameters());

    for (auto& block : transformer_blocks_) {
        add_params_from_raw(block.parameters());
    }

    add_params_from_raw(final_layer_norm_.parameters());

    // No-op deleters for member variables
    params.push_back(std::shared_ptr<TissNum::Parameter>(&output_weight_, [](TissNum::Parameter*){}));
    params.push_back(std::shared_ptr<TissNum::Parameter>(&output_bias_, [](TissNum::Parameter*){}));

    return params;
}

Matrix TransformerModel::forward(const Matrix& input_tokens) {
    // 1. Embedding layer
    std::vector<size_t> token_ids(input_tokens.cols());
    for (size_t i = 0; i < input_tokens.cols(); ++i) {
        token_ids[i] = static_cast<size_t>(input_tokens({0, i}));
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

    // 4. Final Layer Norm
    final_layer_norm_output_ = final_layer_norm_.forward(x);

    // 5. Output Linear Layer
    TissNum::Matrix output_no_bias = TissNum::Matrix::matmul(final_layer_norm_output_, output_weight_.value());
    TissNum::Matrix output({output_no_bias.rows(), output_no_bias.cols()});
    for (size_t r = 0; r < output.rows(); ++r) {
        for (size_t c = 0; c < output.cols(); ++c) {
            output({r, c}) = output_no_bias({r, c}) + output_bias_.value()({0, c});
        }
    }

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
        token_ids_inference[i] = static_cast<size_t>(input_tokens({0, i}));
    }
    TissNum::Matrix x = embedding_layer_.forward(token_ids_inference);

    // 2. Positional Encoding
    size_t current_position = 0;
    if (!past_kv_cache.empty() && past_kv_cache[0].first.rows() > 0) {
        current_position = past_kv_cache[0].first.rows();
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

    // 4. Final Layer Norm
    x = final_layer_norm_.forward(x);

    // 5. Output Linear Layer
    TissNum::Matrix output_no_bias = TissNum::Matrix::matmul(x, output_weight_.value());
    TissNum::Matrix output({output_no_bias.rows(), output_no_bias.cols()});
    for (size_t r = 0; r < output.rows(); ++r) {
        for (size_t c = 0; c < output.cols(); ++c) {
            output({r, c}) = output_no_bias({r, c}) + output_bias_.value()({0, c});
        }
    }

    return output;
}

std::vector<TissNum::Parameter*> TransformerModel::get_parameters() {
    std::vector<TissNum::Parameter*> params;

    auto add_params_from_raw = [&](const std::vector<TissNum::Parameter*>& raw_params) {
        params.insert(params.end(), raw_params.begin(), raw_params.end());
    };

    add_params_from_raw(embedding_layer_.parameters());

    for (auto& block : transformer_blocks_) {
        add_params_from_raw(block.parameters());
    }

    add_params_from_raw(final_layer_norm_.parameters());

    params.push_back(&output_weight_);
    params.push_back(&output_bias_);

    return params;
}

const TissNum::Matrix& TransformerModel::get_embeddings() const {
    return embedding_layer_.get_weight();
}

std::vector<std::vector<float>> TransformerModel::get_embeddings_as_vectors() const {
    const auto& embedding_matrix = embedding_layer_.get_weight();
    std::vector<std::vector<float>> embeddings;
    embeddings.reserve(embedding_matrix.rows());
    for (size_t i = 0; i < embedding_matrix.rows(); ++i) {
        std::vector<float> row;
        row.reserve(embedding_matrix.cols());
        for (size_t j = 0; j < embedding_matrix.cols(); ++j) {
            row.push_back(embedding_matrix(i, j));
        }
        embeddings.push_back(row);
    }
    return embeddings;
}

} // namespace Core
} // namespace TissLM

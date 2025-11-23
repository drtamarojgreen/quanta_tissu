#include "transformer_model.h"
#include <limits>

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
        token_ids[i] = static_cast<size_t>(input_tokens({ {0, i} }));
    }
    cached_token_ids_ = token_ids; // Store for backward pass
    embedded_input_ = embedding_layer_.forward(token_ids);

    // 2. Positional Encoding
    Matrix x_2d = positional_encoding_layer_.forward(embedded_input_);

    // Create a new 3D matrix with a batch size of 1
    Matrix x({1, x_2d.rows(), x_2d.cols()});
    for (size_t r = 0; r < x_2d.rows(); ++r) {
        for (size_t c = 0; c < x_2d.cols(); ++c) {
            x({ {0, r, c} }) = x_2d({ {r, c} });
        }
    }

    // Create causal mask
    size_t seq_len = x.get_shape()[1];
    Matrix mask = Matrix::zeros({1, 1, seq_len, seq_len});
    float neg_inf = -std::numeric_limits<float>::infinity();
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = i + 1; j < seq_len; ++j) {
            mask({0, 0, i, j}) = neg_inf;
        }
    }

    // 3. Transformer Blocks
    transformer_block_outputs_.clear();
    transformer_block_outputs_.push_back(x); // Store input to first block
    for (auto& block : transformer_blocks_) {
        x = block.forward(x, mask);
        transformer_block_outputs_.push_back(x); // Store output of each block
    }

    // 4. Final Layer Norm
    final_layer_norm_output_ = final_layer_norm_.forward(x);

    // 5. Output Linear Layer
    TissNum::Matrix output_3d = TissNum::Matrix::matmul(final_layer_norm_output_, output_weight_.value());
    output_3d = output_3d + output_bias_.value(); // Broadcasting addition

    // Remove batch dimension to return a 2D matrix
    Matrix output_2d({output_3d.get_shape()[1], output_3d.get_shape()[2]});
    for (size_t r = 0; r < output_2d.rows(); ++r) {
        for (size_t c = 0; c < output_2d.cols(); ++c) {
            output_2d({ {r, c} }) = output_3d({ {0, r, c} });
        }
    }

    return output_2d;
}

Matrix TransformerModel::forward_inference(const Matrix& input_tokens, const std::vector<std::pair<Matrix, Matrix>>& past_kv_cache, std::vector<std::pair<Matrix, Matrix>>& new_kv_cache) {
    // Ensure new_kv_cache is correctly sized
    if (new_kv_cache.size() != num_layers_) {
        new_kv_cache.resize(num_layers_);
    }

    // 1. Embedding layer
    std::vector<size_t> token_ids_inference(input_tokens.cols());
    for (size_t i = 0; i < input_tokens.cols(); ++i) {
        token_ids_inference[i] = static_cast<size_t>(input_tokens({ {0, i} }));
    }
    TissNum::Matrix x_2d = embedding_layer_.forward(token_ids_inference);

    // 2. Positional Encoding
    size_t current_position = 0;
    if (!past_kv_cache.empty() && !past_kv_cache[0].first.get_shape().empty()) {
        current_position = past_kv_cache[0].first.get_shape()[2];
    }
    x_2d = positional_encoding_layer_.forward(x_2d, current_position);

    // Create a new 3D matrix with a batch size of 1
    Matrix x({1, x_2d.rows(), x_2d.cols()});
    for (size_t r = 0; r < x_2d.rows(); ++r) {
        for (size_t c = 0; c < x_2d.cols(); ++c) {
            x({ {0, r, c} }) = x_2d({ {r, c} });
        }
    }

    // Create mask if processing more than one token
    size_t seq_len = x.get_shape()[1];
    Matrix mask;
    if (seq_len > 1) {
        size_t past_len = 0;
        if (!past_kv_cache.empty() && !past_kv_cache[0].first.get_shape().empty()) {
            past_len = past_kv_cache[0].first.get_shape()[2];
        }
        size_t total_len = past_len + seq_len;
        
        mask = Matrix::zeros({1, 1, seq_len, total_len});
        float neg_inf = -std::numeric_limits<float>::infinity();
        
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = past_len + i + 1; j < total_len; ++j) {
                mask({0, 0, i, j}) = neg_inf;
            }
        }
    }

    // 3. Transformer Blocks
    for (int i = 0; i < num_layers_; ++i) {
        std::optional<std::pair<Matrix, Matrix>> current_past_kv = std::nullopt;
        if (i < past_kv_cache.size()) {
            current_past_kv = past_kv_cache[i];
        }
        
        std::optional<std::pair<Matrix, Matrix>> current_new_kv;
        x = transformer_blocks_[i].forward(x, mask, current_past_kv, &current_new_kv);
        
        if (current_new_kv.has_value()) {
            new_kv_cache[i] = current_new_kv.value();
        }
    }

    // 4. Final Layer Norm
    x = final_layer_norm_.forward(x);

    // 5. Output Linear Layer
    TissNum::Matrix output_3d = TissNum::Matrix::matmul(x, output_weight_.value());
    output_3d = output_3d + output_bias_.value(); // Broadcasting addition

    // Remove batch dimension to return a 2D matrix
    Matrix output_2d({output_3d.get_shape()[1], output_3d.get_shape()[2]});
    for (size_t r = 0; r < output_2d.rows(); ++r) {
        for (size_t c = 0; c < output_2d.cols(); ++c) {
            output_2d({ {r, c} }) = output_3d({ {0, r, c} });
        }
    }

    return output_2d;
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
            row.push_back(embedding_matrix({ {i, j} }));
        }
        embeddings.push_back(row);
    }
    return embeddings;
}

} // namespace Core
} // namespace TissLM

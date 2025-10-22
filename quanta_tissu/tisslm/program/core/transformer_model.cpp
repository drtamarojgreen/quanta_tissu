#include "transformer_model.h"

namespace TissDB {
namespace TissLM {
namespace Core {

using namespace TissNum;

TransformerModel::TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, float dropout_rate, int lora_rank)
    : token_embedding_table_(std::in_place, Matrix::random(vocab_size, embed_dim), "token_embedding_table"),
      position_embedding_table_(std::in_place, Matrix::random(max_seq_len, embed_dim), "position_embedding_table"),
      final_layernorm_(embed_dim, "final_layernorm"),
      output_layer_(std::in_place, Matrix::random(embed_dim, vocab_size), "output_layer"),
      vocab_size_(vocab_size),
      embed_dim_(embed_dim),
      num_layers_(num_layers)
{
    layers_.reserve(num_layers);
    size_t d_ff = embed_dim * 4;
    for (int i = 0; i < num_layers; ++i) {
        layers_.emplace_back(embed_dim, num_heads, d_ff, dropout_rate, lora_rank, "layer_" + std::to_string(i));
    }
}

Matrix TransformerModel::forward(const Matrix& input_tokens) {
    // 1. Token Embeddings
    Matrix embedded = Matrix::embedding_lookup(input_tokens, token_embedding_table_->value());

    // 2. Positional Embeddings
    Matrix pos_embeddings = position_embedding_table_->value();
    Matrix x = embedded + pos_embeddings.slice(0, embedded.rows(), 0, embedded.cols());

    // 3. Transformer Blocks
    transformer_block_outputs_.clear();
    transformer_block_outputs_.push_back(x);
    for (auto& block : layers_) {
        x = block.forward(x);
        transformer_block_outputs_.push_back(x);
    }

    // 4. Final LayerNorm
    auto [ln_out, ln_cache] = final_layernorm_.forward(x);
    transformer_block_outputs_.push_back(ln_cache); // Cache for backward pass

    // 5. Output Linear Layer
    Matrix logits = Matrix::matmul(ln_out, output_layer_->value());

    return logits;
}

Matrix TransformerModel::forward_inference(const Matrix& input_tokens, const std::vector<std::pair<Matrix, Matrix>>& past_kv_cache, std::vector<std::pair<Matrix, Matrix>>& new_kv_cache) {
    new_kv_cache.resize(num_layers_);

    // 1. Token Embeddings
    Matrix embedded = Matrix::embedding_lookup(input_tokens, token_embedding_table_->value());

    // 2. Positional Embeddings
    size_t current_pos = past_kv_cache.empty() ? 0 : past_kv_cache[0].first.rows();
    Matrix pos_embeddings = position_embedding_table_->value().slice(current_pos, current_pos + 1, 0, embed_dim_);
    Matrix x = embedded + pos_embeddings;

    // 3. Transformer Blocks
    for (int i = 0; i < num_layers_; ++i) {
        std::optional<std::pair<Matrix, Matrix>> past_kv = (i < past_kv_cache.size()) ? std::make_optional(past_kv_cache[i]) : std::nullopt;
        std::optional<std::pair<Matrix, Matrix>> new_kv;
        x = layers_[i].forward(x, Matrix(), past_kv, &new_kv);
        if (new_kv.has_value()) {
            new_kv_cache[i] = new_kv.value();
        }
    }

    // 4. Final LayerNorm
    auto [ln_out, ln_cache] = final_layernorm_.forward(x);

    // 5. Output Linear Layer
    Matrix logits = Matrix::matmul(ln_out, output_layer_->value());
    return logits;
}

Matrix TransformerModel::backward(const Matrix& grad_output) {
    // 1. Backward through Output Linear Layer
    Matrix grad_x = Matrix::matmul(grad_output, output_layer_->value().transpose());
    output_layer_->grad() = Matrix::matmul(transformer_block_outputs_.back().transpose(), grad_output);

    // 2. Backward through Final LayerNorm
    grad_x = final_layernorm_.backward(grad_x, transformer_block_outputs_.back());
    transformer_block_outputs_.pop_back();


    // 3. Backward through Transformer Blocks
    for (int i = num_layers_ - 1; i >= 0; --i) {
        grad_x = layers_[i].backward(grad_x, transformer_block_outputs_[i]);
    }

    // 4. Backward through Embeddings (sum gradients from token and positional)
    // Note: This is simplified. A real implementation would handle the addition.
    // Also, positional embedding gradients are usually not computed.
    Matrix grad_token_embeddings = grad_x;

    // The gradient for the embedding table requires a special operation.
    // For now, we'll just assign it as a placeholder.
    token_embedding_table_->grad() = Matrix::zeros(vocab_size_, embed_dim_);


    return Matrix();
}

std::vector<std::shared_ptr<TissNum::Parameter>> TransformerModel::get_parameters() {
    std::vector<std::shared_ptr<TissNum::Parameter>> params;

    auto add_param = [&](std::optional<Parameter>& p) {
        if (p.has_value()) {
            params.push_back(std::make_shared<Parameter>(p.value()));
        }
    };

    add_param(token_embedding_table_);
    add_param(position_embedding_table_);

    for (auto& block : layers_) {
        for(auto* p : block.parameters()){
            params.push_back(std::shared_ptr<Parameter>(p, [](Parameter*){}));
        }
    }

    for(auto* p : final_layernorm_.parameters()){
        params.push_back(std::shared_ptr<Parameter>(p, [](Parameter*){}));
    }

    add_param(output_layer_);

    return params;
}

const TissNum::Matrix& TransformerModel::get_embeddings() const {
    return token_embedding_table_->value();
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

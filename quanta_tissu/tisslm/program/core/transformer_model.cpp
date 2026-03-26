#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include <limits>
#include <cmath>

namespace TissLM {
namespace Core {

using namespace TissNum;

TransformerModel::TransformerModel(int vocab_size, int max_seq_len, int embed_dim, int num_heads, int num_layers, int d_ff, float dropout_rate, int lora_rank)
    : embedding_layer_(vocab_size, embed_dim),
      positional_encoding_layer_(embed_dim, max_seq_len),
      final_layer_norm_(embed_dim),
      output_weight_(TissNum::Matrix::random({(size_t)embed_dim, (size_t)vocab_size}, 0.0f, 1.0f / std::sqrt((float)embed_dim)), "output_weight"),
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
    size_t seq_len = final_layer_norm_output_.get_shape()[1];
    size_t embed_dim = final_layer_norm_output_.get_shape()[2];
    TissNum::Matrix x_2d = final_layer_norm_output_.reshape({seq_len, embed_dim});
    output_weight_.grad() = TissNum::Matrix::matmul(x_2d.transpose(), grad_output);
    output_bias_.grad() = grad_output.sum(0);
    TissNum::Matrix grad_x = TissNum::Matrix::matmul(grad_output, output_weight_.value().transpose());
    grad_x = final_layer_norm_.backward(grad_x);
    for (int i = num_layers_ - 1; i >= 0; --i) grad_x = transformer_blocks_[i].backward(grad_x);
    if (grad_x.get_shape().size() == 3) {
        size_t sl = grad_x.get_shape()[1], ed = grad_x.get_shape()[2];
        grad_x = grad_x.reshape({sl, ed});
    }
    embedding_layer_.backward(grad_x, cached_token_ids_);
}

std::vector<std::shared_ptr<TissNum::Parameter>> TransformerModel::get_parameters() {
    std::vector<std::shared_ptr<TissNum::Parameter>> params;
    auto add_raw = [&](const std::vector<TissNum::Parameter*>& raw) {
        for (auto* p : raw) params.push_back(std::shared_ptr<TissNum::Parameter>(p, [](TissNum::Parameter*){}));
    };
    add_raw(embedding_layer_.parameters());
    for (auto& block : transformer_blocks_) add_raw(block.parameters());
    add_raw(final_layer_norm_.parameters());
    params.push_back(std::shared_ptr<TissNum::Parameter>(&output_weight_, [](TissNum::Parameter*){}));
    params.push_back(std::shared_ptr<TissNum::Parameter>(&output_bias_, [](TissNum::Parameter*){}));
    return params;
}

Matrix TransformerModel::forward(const Matrix& input_tokens, bool training) {
    std::vector<size_t> token_ids(input_tokens.cols());
    for (size_t i = 0; i < input_tokens.cols(); ++i) token_ids[i] = static_cast<size_t>(input_tokens({ 0, i }));
    cached_token_ids_ = token_ids;
    embedded_input_ = embedding_layer_.forward(token_ids);
    Matrix x_2d = positional_encoding_layer_.forward(embedded_input_);
    Matrix x({1, x_2d.rows(), x_2d.cols()});
    for (size_t r = 0; r < x_2d.rows(); ++r) for (size_t c = 0; c < x_2d.cols(); ++c) x({ 0, r, c }) = x_2d({ r, c });
    size_t seq_len = x.get_shape()[1];
    Matrix mask = Matrix::zeros({1, 1, seq_len, seq_len});
    for (size_t i = 0; i < seq_len; ++i) for (size_t j = i + 1; j < seq_len; ++j) mask({ 0, 0, i, j }) = -1e9;
    transformer_block_outputs_.clear(); transformer_block_outputs_.push_back(x);
    for (auto& block : transformer_blocks_) {
        x = block.forward(x, mask, std::nullopt, nullptr, training);
        transformer_block_outputs_.push_back(x);
    }
    final_layer_norm_output_ = final_layer_norm_.forward(x);
    TissNum::Matrix out_3d = TissNum::Matrix::matmul(final_layer_norm_output_, output_weight_.value()) + output_bias_.value();
    Matrix out_2d({out_3d.get_shape()[1], out_3d.get_shape()[2]});
    for (size_t r = 0; r < out_2d.rows(); ++r) for (size_t c = 0; c < out_2d.cols(); ++c) out_2d({ r, c }) = out_3d({ 0, r, c });
    return out_2d;
}

Matrix TransformerModel::forward_inference(const Matrix& input_tokens, const std::vector<std::pair<Matrix, Matrix>>& past_kv, std::vector<std::pair<Matrix, Matrix>>& new_kv) {
    if (new_kv.size() != num_layers_) new_kv.resize(num_layers_);
    std::vector<size_t> ids(input_tokens.cols());
    for (size_t i = 0; i < input_tokens.cols(); ++i) ids[i] = static_cast<size_t>(input_tokens({ 0, i }));
    TissNum::Matrix x_2d = embedding_layer_.forward(ids);
    size_t pos = 0; if (!past_kv.empty() && !past_kv[0].first.get_shape().empty()) pos = past_kv[0].first.get_shape()[2];
    x_2d = positional_encoding_layer_.forward(x_2d, pos);
    Matrix x({1, x_2d.rows(), x_2d.cols()});
    for (size_t r = 0; r < x_2d.rows(); ++r) for (size_t c = 0; c < x_2d.cols(); ++c) x({ 0, r, c }) = x_2d({ r, c });
    size_t seq_len = x.get_shape()[1];
    Matrix mask;
    if (seq_len > 1) {
        size_t past_len = 0; if (!past_kv.empty() && !past_kv[0].first.get_shape().empty()) past_len = past_kv[0].first.get_shape()[2];
        size_t tot_len = past_len + seq_len; mask = Matrix::zeros({1, 1, seq_len, tot_len});
        for (size_t i = 0; i < seq_len; ++i) for (size_t j = past_len + i + 1; j < tot_len; ++j) mask({ 0, 0, i, j }) = -1e9;
    }
    for (int i = 0; i < num_layers_; ++i) {
        std::optional<std::pair<Matrix, Matrix>> pkv = std::nullopt; if (i < (int)past_kv.size()) pkv = past_kv[i];
        std::optional<std::pair<Matrix, Matrix>> nkv;
        x = transformer_blocks_[i].forward(x, mask, pkv, &nkv);
        if (nkv.has_value()) new_kv[i] = nkv.value();
    }
    x = final_layer_norm_.forward(x);
    TissNum::Matrix out_3d = TissNum::Matrix::matmul(x, output_weight_.value()) + output_bias_.value();
    Matrix out_2d({out_3d.get_shape()[1], out_3d.get_shape()[2]});
    for (size_t r = 0; r < out_2d.rows(); ++r) for (size_t c = 0; c < out_2d.cols(); ++c) out_2d({ r, c }) = out_3d({ 0, r, c });
    return out_2d;
}

const TissNum::Matrix& TransformerModel::get_embeddings() const { return embedding_layer_.get_weight(); }

std::vector<std::vector<float>> TransformerModel::get_embeddings_as_vectors() const {
    const auto& em = embedding_layer_.get_weight();
    std::vector<std::vector<float>> res; res.reserve(em.rows());
    for (size_t i = 0; i < em.rows(); ++i) {
        std::vector<float> row; row.reserve(em.cols());
        for (size_t j = 0; j < em.cols(); ++j) row.push_back(em({ i, j }));
        res.push_back(row);
    }
    return res;
}

}
}

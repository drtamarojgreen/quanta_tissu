#include "model.h"
#include <cmath>

namespace cllm {

Model::Model(const ModelConfig& config) : config_(config) {
    // Initialize token embeddings
    token_embeddings_ = Eigen::MatrixXf::Random(config.vocab_size, config.d_model);

    // Initialize transformer blocks
    layers_.reserve(config.n_layers);
    for (int i = 0; i < config.n_layers; ++i) {
        layers_.push_back(std::make_unique<TransformerBlock>(config));
    }

    // Initialize final output layer
    output_layer_weight_ = Eigen::MatrixXf::Random(config.vocab_size, config.d_model);
}

void Model::add_positional_encoding(Eigen::MatrixXf& input) {
    int seq_len = input.rows();
    int d_model = input.cols();

    Eigen::MatrixXf pos_encoding(seq_len, d_model);
    for (int pos = 0; pos < seq_len; ++pos) {
        for (int i = 0; i < d_model / 2; ++i) {
            double div_term = std::pow(10000.0, (2.0 * i) / d_model);
            pos_encoding(pos, 2 * i) = std::sin(pos / div_term);
            pos_encoding(pos, 2 * i + 1) = std::cos(pos / div_term);
        }
    }
    input += pos_encoding;
}

Eigen::MatrixXf Model::forward(const std::vector<int>& input_tokens) {
    int seq_len = input_tokens.size();

    // 1. Token Embedding
    Eigen::MatrixXf x(seq_len, config_.d_model);
    for (int i = 0; i < seq_len; ++i) {
        x.row(i) = token_embeddings_.row(input_tokens[i]);
    }

    // 2. Add Positional Encoding
    add_positional_encoding(x);

    // 3. Pass through Transformer Blocks
    for (auto& layer : layers_) {
        x = layer->forward(x);
    }

    // 4. Final projection to logits
    Eigen::MatrixXf logits = x * output_layer_weight_.transpose();

    return logits;
}

} // namespace cllm

#include "model.h"
#include <cmath>

Model::Model(int vocab_size, int d_model, int n_layer, int n_head, int d_ff)
    : embeddings(Matrix::random(vocab_size, d_model)),
      output_proj(Matrix::random(d_model, vocab_size)) {
    for (int i = 0; i < n_layer; ++i) {
        transformer_blocks.emplace_back(d_model, n_head, d_ff);
    }
    positional_encoding = create_positional_encoding(5000, d_model); // Max length of 5000
}

Matrix Model::create_positional_encoding(int max_len, int d_model) {
    Matrix pe(max_len, d_model);
    for (int pos = 0; pos < max_len; ++pos) {
        for (int i = 0; i < d_model; i += 2) {
            float div_term = std::pow(10000.0, static_cast<float>(i) / d_model);
            pe.at(pos, i) = std::sin(pos / div_term);
            if (i + 1 < d_model) {
                pe.at(pos, i + 1) = std::cos(pos / div_term);
            }
        }
    }
    return pe;
}

Matrix Model::forward(const std::vector<int>& token_ids) {
    int seq_len = token_ids.size();
    int d_model = embeddings.get_cols();

    // Create input embeddings
    Matrix x(seq_len, d_model);
    for (int i = 0; i < seq_len; ++i) {
        for (int j = 0; j < d_model; ++j) {
            x.at(i, j) = embeddings.at(token_ids[i], j);
        }
    }

    // Add positional encoding
    for (int i = 0; i < seq_len; ++i) {
        for (int j = 0; j < d_model; ++j) {
            x.at(i, j) += positional_encoding.at(i, j);
        }
    }

    // Pass through transformer blocks
    for (auto& block : transformer_blocks) {
        x = block.forward(x);
    }

    // Output projection
    return x.dot(output_proj);
}
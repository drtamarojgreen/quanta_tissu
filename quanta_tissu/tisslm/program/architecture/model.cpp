#include "model.h"
#include <cmath>

Model::Model(int vocab_size, int d_model, int n_layer, int n_head, int d_ff)
    : embeddings(Matrix::random(std::vector<size_t>{(size_t)vocab_size, (size_t)d_model})),
      output_proj(Matrix::random(std::vector<size_t>{(size_t)d_model, (size_t)vocab_size})),
      d_embeddings(vocab_size, d_model),
      d_output_proj(d_model, vocab_size) {
    for (int i = 0; i < n_layer; ++i) {
        transformer_blocks.emplace_back(d_model, n_head, d_ff);
    }
    positional_encoding = create_positional_encoding(5000, d_model); // Max length of 5000
}

Matrix Model::create_positional_encoding(int max_len, int d_model) {
    Matrix pe(std::vector<size_t>{(size_t)max_len, (size_t)d_model});
    for (int pos = 0; pos < max_len; ++pos) {
        for (int i = 0; i < d_model; i += 2) {
            float div_term = std::pow(10000.0f, static_cast<float>(i) / d_model);
            pe({(size_t)pos, (size_t)i}) = std::sin(pos / div_term);
            if (i + 1 < (size_t)d_model) {
                pe({(size_t)pos, (size_t)i + 1}) = std::cos(pos / div_term);
            }
        }
    }
    return pe;
}

Matrix Model::forward(const std::vector<int>& token_ids) {
    size_t seq_len = token_ids.size();
    size_t d_model = embeddings.cols();

    // Create input embeddings
    Matrix x(std::vector<size_t>{seq_len, d_model});
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            x({i, j}) = embeddings({(size_t)token_ids[i], j});
        }
    }

    // Add positional encoding
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            x({i, j}) = x({i, j}) + positional_encoding({i, j});
        }
    }

    // Pass through transformer blocks
    for (auto& block : transformer_blocks) {
        x = block.forward(x);
    }

    // Output projection
    return Matrix::matmul(x, output_proj);
}

void Model::backward(const Matrix& d_logits, const std::vector<int>& token_ids) {
    size_t seq_len = token_ids.size();
    size_t d_model = embeddings.cols();

    // Re-run forward to get activations (in-place for simplicity in this demo)
    Matrix x(seq_len, d_model);
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = 0; j < d_model; ++j) x({i, j}) = embeddings({(size_t)token_ids[i], j}) + positional_encoding({i, j});
    }
    std::vector<Matrix> block_inputs;
    block_inputs.push_back(x);
    for (auto& block : transformer_blocks) {
        x = block.forward(x);
        block_inputs.push_back(x);
    }

    // Backprop through output projection
    d_output_proj = d_output_proj + Matrix::matmul(x.transpose(), d_logits);
    Matrix dx = Matrix::matmul(d_logits, output_proj.transpose());

    // Backprop through blocks
    for (int i = (int)transformer_blocks.size() - 1; i >= 0; --i) {
        dx = transformer_blocks[i].backward(dx, block_inputs[i]);
    }

    // Backprop through embeddings
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            d_embeddings({(size_t)token_ids[i], j}) += dx({i, j});
        }
    }
}

std::vector<Matrix*> Model::parameters() {
    std::vector<Matrix*> p = {&embeddings, &output_proj};
    for (auto& block : transformer_blocks) {
        auto bp = block.get_parameters();
        p.insert(p.end(), bp.begin(), bp.end());
    }
    return p;
}

std::vector<Matrix*> Model::gradients() {
    std::vector<Matrix*> g = {&d_embeddings, &d_output_proj};
    for (auto& block : transformer_blocks) {
        auto bg = block.get_gradients();
        g.insert(g.end(), bg.begin(), bg.end());
    }
    return g;
}

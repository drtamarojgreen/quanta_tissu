#include "model.h"
#include <cmath>

Model::Model(int vocab_size, int d_model, int n_layer, int n_head, int d_ff, AttentionMode attention_mode)
    : embeddings(Matrix({vocab_size, d_model})),
      output_proj(Matrix({d_model, vocab_size})) {
    for (int i = 0; i < n_layer; ++i) {
        transformer_blocks.emplace_back(d_model, n_head, d_ff, attention_mode);
    }
    positional_encoding = create_positional_encoding(5000, d_model); // Max length of 5000
}

Matrix Model::create_positional_encoding(int max_len, int d_model) {
    Matrix pe({max_len, d_model});
    // Placeholder for actual positional encoding logic with the new Matrix class
    return pe;
}

Matrix Model::forward(const std::vector<int>& token_ids) {
    int seq_len = token_ids.size();

    Matrix x({seq_len, (int)embeddings.shape()[1]});
    // Placeholder for embedding lookup and positional encoding addition

    // Pass through transformer blocks
    for (auto& block : transformer_blocks) {
        x = block.forward(x);
    }

    // Output projection (conceptual)
    // return matmul(x, output_proj);
    return x;
}
#include "quantatissu.h"
#include <iostream>
#include <vector>
#include <algorithm>

QuantaTissu::QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix)
    : model_(std::make_shared<TissLM::Core::TransformerModel>(
        config.vocab_size, config.max_seq_len, config.embed_dim,
        config.num_heads, config.num_layers, config.d_ff, config.dropout_rate)),
      tokenizer_(tokenizer_prefix) {}

std::string QuantaTissu::generate(const std::string& prompt, int n_new_tokens) {
    std::vector<int> token_ids = tokenizer_.encode(prompt);

    for (int i = 0; i < n_new_tokens; ++i) {
        // Convert to matrix for core model
        TissNum::Matrix input({1, token_ids.size()});
        for (size_t j = 0; j < token_ids.size(); ++j) input({0, j}) = static_cast<float>(token_ids[j]);

        TissNum::Matrix logits = model_->forward(input, false);

        // Greedy sampling from the last position
        int next_token_id = 0;
        float max_logit = -1e9f;
        size_t last_row = logits.rows() - 1;
        for (size_t j = 0; j < logits.cols(); ++j) {
            if (logits({last_row, j}) > max_logit) {
                max_logit = logits({last_row, j});
                next_token_id = static_cast<int>(j);
            }
        }
        token_ids.push_back(next_token_id);

        // Safety break if EOS (assuming 0 is EOS or we just stop at limit)
        if (next_token_id == 0) break;
    }

    return tokenizer_.decode(token_ids);
}

void QuantaTissu::load_weights(const std::string& path) {
    std::cout << "Unified Weight Loading: Not yet implemented for path " << path << std::endl;
}

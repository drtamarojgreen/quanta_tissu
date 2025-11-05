#include "quantatissu.h"
#include <iostream>
#include <vector>
#include <algorithm>

QuantaTissu::QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix)
    : model(config.vocab_size, config.d_model, config.n_layer, config.n_head, config.d_ff),
      tokenizer(tokenizer_prefix) {}

std::string QuantaTissu::generate(const std::string& prompt, int n_new_tokens) {
    std::vector<int> token_ids = tokenizer.encode(prompt);

    for (int i = 0; i < n_new_tokens; ++i) {
        Matrix logits = model.forward(token_ids);

        // For simplicity, we'll use greedy sampling.
        int next_token_id = 0;
        float max_logit = -1e9;
        for (size_t j = 0; j < logits.cols(); ++j) {
            if (logits({logits.rows() - 1, j}) > max_logit) {
                max_logit = logits({logits.rows() - 1, j});
                next_token_id = j;
            }
        }
        token_ids.push_back(next_token_id);
    }

    return tokenizer.decode(token_ids);
}

void QuantaTissu::load_weights(const std::string& path) {
    // Placeholder for loading pre-trained model weights.
    // This would involve reading a file (e.g., in NumPy's .npz format or a custom binary format)
    // and populating the weight matrices in the Model and its sub-layers.
    std::cout << "Weight loading is not yet implemented." << std::endl;
}
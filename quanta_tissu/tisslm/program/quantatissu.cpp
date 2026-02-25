#include "quantatissu.h"
#include <iostream>
#include <cmath>
#include <algorithm>

QuantaTissu::QuantaTissu() {
    config_.d_model = 128;
    config_.num_layers = 2;
    config_.num_heads = 4;
    config_.d_ff = 512;
    config_.vocab_size = 1000;

    tokenizer_ = std::make_unique<TissLM::Tokenizer::Tokenizer>("");
    model_ = std::make_unique<Model>(config_.vocab_size, config_.d_model, config_.num_layers, config_.num_heads, config_.d_ff);
}

QuantaTissu::QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix)
    : config_(config)
{
    tokenizer_ = std::make_unique<TissLM::Tokenizer::Tokenizer>(tokenizer_prefix);
    model_ = std::make_unique<Model>(config.vocab_size, config.d_model, config.num_layers, config.num_heads, config.d_ff);
}

void QuantaTissu::train_tokenizer(const std::string& text) {
    std::cout << "[QuantaTissu] Training tokenizer on " << text.length() << " chars..." << std::endl;
    tokenizer_->train(text, config_.vocab_size, false);
    tokenizer_->save("stellar");
}

void QuantaTissu::train_step(const std::string& text, float& loss) {
    auto tokens = tokenizer_->encode(text);
    if (tokens.size() < 2) {
        loss = 0.0f;
        return;
    }

    std::vector<int> inputs(tokens.begin(), tokens.end() - 1);
    std::vector<int> targets(tokens.begin() + 1, tokens.end());

    Matrix logits = model_->forward(inputs);

    // Cross-Entropy Loss calculation
    float total_loss = 0.0f;
    for (size_t i = 0; i < inputs.size(); ++i) {
        float max_l = logits({i, 0});
        for (size_t v = 1; v < (size_t)config_.vocab_size; ++v) {
            max_l = std::max(max_l, logits({i, v}));
        }

        float sum_exp = 0.0f;
        for (size_t v = 0; v < (size_t)config_.vocab_size; ++v) {
            sum_exp += std::exp(logits({i, v}) - max_l);
        }

        float target_logit = logits({i, (size_t)targets[i]});
        float log_prob = target_logit - max_l - std::log(sum_exp);
        total_loss -= log_prob;
    }
    loss = total_loss / inputs.size();
}

void QuantaTissu::train(const std::string& text, float& final_loss) {
    train_step(text, final_loss);
    // Note: Parameter updates (SGD) are currently not implemented as the
    // indigenous Matrix lacks autograd, but the loss calculation is now real.
}

std::string QuantaTissu::generate(const std::string& prompt, size_t max_len) {
    auto tokens = tokenizer_->encode(prompt);
    size_t original_size = tokens.size();

    for (size_t i = 0; i < max_len; ++i) {
        Matrix logits = model_->forward(tokens);
        size_t last_token_idx = tokens.size() - 1;

        int next_token = 0;
        float max_logit = -1e9;
        for (int v = 0; v < config_.vocab_size; ++v) {
            if (logits({last_token_idx, (size_t)v}) > max_logit) {
                max_logit = logits({last_token_idx, (size_t)v});
                next_token = v;
            }
        }

        tokens.push_back(next_token);
        if (next_token == 0 && i > 5) break; // Heuristic EOS
    }

    return tokenizer_->decode(tokens);
}

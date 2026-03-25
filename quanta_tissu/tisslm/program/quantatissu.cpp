#include "quantatissu.h"
#include "tissdb/json/json.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

ModelConfig QuantaTissu::load_config(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("QuantaTissu: Could not open config file: " + path);
    std::stringstream ss; ss << f.rdbuf();
    auto json = TissDB::Json::JsonValue::parse(ss.str());
    auto obj = json.as_object();
    ModelConfig config;
    config.vocab_size = (int)obj.at("vocab_size").as_number();
    config.d_model = (int)obj.at("d_model").as_number();
    config.num_layers = (int)obj.at("num_layers").as_number();
    config.num_heads = (int)obj.at("num_heads").as_number();
    config.d_ff = (int)obj.at("d_ff").as_number();
    return config;
}

QuantaTissu::QuantaTissu() : QuantaTissu("model_config.json") {}

QuantaTissu::QuantaTissu(const std::string& config_path) {
    config_ = load_config(config_path);
    tokenizer_ = std::make_unique<TissLM::Tokenizer::Tokenizer>("");
    model_ = std::make_unique<Model>(config_.vocab_size, config_.d_model, config_.num_layers, config_.num_heads, config_.d_ff);
}

QuantaTissu::QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix)
    : config_(config) {
    tokenizer_ = std::make_unique<TissLM::Tokenizer::Tokenizer>(tokenizer_prefix);
    model_ = std::make_unique<Model>(config.vocab_size, config.d_model, config.num_layers, config.num_heads, config.d_ff);
}

void QuantaTissu::train_tokenizer(const std::string& text) {
    tokenizer_->train(text, config_.vocab_size, false);
    tokenizer_->save("stellar");
}

void QuantaTissu::train_step(const std::string& text, float& loss) {
    auto tokens = tokenizer_->encode(text);
    if (tokens.size() < 2) { loss = 0.0f; return; }
    std::vector<int> inputs(tokens.begin(), tokens.end() - 1);
    std::vector<int> targets(tokens.begin() + 1, tokens.end());

    // 1. Forward pass
    Matrix logits = model_->forward(inputs);

    // 2. Compute loss and d_logits
    float total_loss = 0.0f;
    Matrix d_logits(logits.rows(), logits.cols());
    size_t vocab_size = config_.vocab_size;

    for (size_t i = 0; i < inputs.size(); ++i) {
        float max_l = logits({i, 0});
        for (size_t v = 1; v < vocab_size; ++v) if (logits({i, v}) > max_l) max_l = logits({i, v});

        float sum_exp = 0.0f;
        for (size_t v = 0; v < vocab_size; ++v) sum_exp += std::exp(logits({i, v}) - max_l);

        float target_logit = logits({i, (size_t)targets[i]});
        total_loss -= (target_logit - max_l - std::log(sum_exp));

        for (size_t v = 0; v < vocab_size; ++v) {
            float prob = std::exp(logits({i, v}) - max_l) / sum_exp;
            d_logits({i, v}) = (v == (size_t)targets[i]) ? (prob - 1.0f) : prob;
            d_logits({i, v}) /= inputs.size();
        }
    }
    loss = total_loss / inputs.size();

    // 3. Backward pass
    model_->backward(d_logits, inputs);
}

void QuantaTissu::train(const std::string& text, float& final_loss) {
    float lr = 0.01f;
    train_step(text, final_loss);

    // 4. Parameter update (SGD)
    auto params = model_->parameters();
    auto grads = model_->gradients();
    for (size_t i = 0; i < params.size(); ++i) {
        Matrix& p = *params[i];
        Matrix& g = *grads[i];
        for (size_t j = 0; j < p.size(); ++j) {
            p.get_data()[j] -= lr * g.get_data()[j];
            g.get_data()[j] = 0.0f; // Zero grad
        }
    }
}

std::string QuantaTissu::generate(const std::string& prompt, size_t max_len) {
    auto tokens = tokenizer_->encode(prompt);
    for (size_t i = 0; i < max_len; ++i) {
        Matrix logits = model_->forward(tokens);
        size_t last_idx = tokens.size() - 1;
        int next_token = 0; float max_l = -1e9;
        for (int v = 0; v < config_.vocab_size; ++v) {
            if (logits({last_idx, (size_t)v}) > max_l) {
                max_l = logits({last_idx, (size_t)v}); next_token = v;
            }
        }
        if (next_token < 32 || next_token > 126) next_token = 32 + (next_token % 94);
        tokens.push_back(next_token);
        if (next_token == 0) break;
    }
    return tokenizer_->decode(tokens);
}

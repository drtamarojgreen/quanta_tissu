#ifndef QUANTATISSU_H
#define QUANTATISSU_H

#include "core/transformer_model.h"
#include "tokenizer/tokenizer.h"
#include <string>
#include <vector>
#include <memory>

// Configuration for the unified program
struct ModelConfig {
    int vocab_size;
    int max_seq_len;
    int embed_dim;
    int num_heads;
    int num_layers;
    int d_ff;
    float dropout_rate;
};

class QuantaTissu {
public:
    QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix);

    std::string generate(const std::string& prompt, int n_new_tokens);
    void load_weights(const std::string& path);

    // Accessors for integration
    std::shared_ptr<TissLM::Core::TransformerModel> get_model() { return model_; }
    TissLM::Tokenizer::Tokenizer& get_tokenizer() { return tokenizer_; }

private:
    std::shared_ptr<TissLM::Core::TransformerModel> model_;
    TissLM::Tokenizer::Tokenizer tokenizer_;
};

#endif // QUANTATISSU_H

#ifndef QUANTATISSU_H
#define QUANTATISSU_H

#include <string>
#include <vector>
#include <memory>
#include "architecture/model.h"
#include "tokenizer/tokenizer.h"

struct ModelConfig {
    int vocab_size;
    int d_model;
    int num_layers;
    int num_heads;
    int d_ff;
};

/**
 * @brief Indigenous QuantaTissu facade.
 * Orchestrates the TissLM::Tokenizer and the indigenous Model.
 */
class QuantaTissu {
public:
    QuantaTissu(); // Default constructor for testing
    QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix);

    void train(const std::string& text, float& final_loss);
    std::string generate(const std::string& prompt, size_t max_len = 50);

    // Helpers for the Stellar showcase
    void train_tokenizer(const std::string& text);
    void train_step(const std::string& text, float& loss);

    TissLM::Tokenizer::Tokenizer& get_tokenizer() { return *tokenizer_; }
    Model& get_model() { return *model_; }

private:
    std::unique_ptr<TissLM::Tokenizer::Tokenizer> tokenizer_;
    std::unique_ptr<Model> model_;
    ModelConfig config_;
};

#endif // QUANTATISSU_H

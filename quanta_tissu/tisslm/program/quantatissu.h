#ifndef QUANTATISSU_H
#define QUANTATISSU_H

#include "architecture/model.h"
#include "tokenizer/tokenizer.h"
#include <string>
#include <vector>
#include <map>

// A simple config structure
struct ModelConfig {
    int vocab_size;
    int d_model;
    int n_layer;
    int n_head;
    int d_ff;
};

class QuantaTissu {
public:
    QuantaTissu(const ModelConfig& config, const std::string& tokenizer_prefix);

    std::string generate(const std::string& prompt, int n_new_tokens);
    void load_weights(const std::string& path); // Placeholder for weight loading

private:
    Model model;
    Tokenizer tokenizer;
};

#endif // QUANTATISSU_H
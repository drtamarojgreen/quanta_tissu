#include "ctisslm.h"
#include <iostream>
#include <map>

namespace quanta_tissu {

ctisslm::ctisslm() : is_model_loaded(false), is_tokenizer_loaded(false), model(nullptr) {
    tokenizer_instance = new Tokenizer();
}

ctisslm::~ctisslm() {
    delete model;
    delete tokenizer_instance;
}

bool ctisslm::load_model(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;
    std::stringstream ss; ss << ifs.rdbuf(); ifs.close();
    try {
        auto root = TissDB::Json::JsonValue::parse(ss.str());
        auto config_obj = root.as_object().at("config").as_object();
        std::map<std::string, int> config;
        for (const auto& pair : config_obj) config[pair.first] = (int)pair.second.as_number();

        if (model) delete model;
        model = new QuantaTissuModel(config);

        auto weights = root.as_object().at("weights").as_object();
        auto load_param = [&](Parameter& p, const std::string& name) {
            auto arr = weights.at(name).as_array();
            for (size_t i = 0; i < p.value.size(); ++i) p.value[i] = arr[i].as_number();
        };
        load_param(model->embeddings, "embeddings");
        load_param(model->output_proj, "output_proj");
        for (size_t i = 0; i < model->transformer_blocks.size(); ++i) {
            std::string b = "blk." + std::to_string(i);
            load_param(model->transformer_blocks[i].mha.Wq, b + ".mha.Wq");
            load_param(model->transformer_blocks[i].mha.Wk, b + ".mha.Wk");
            load_param(model->transformer_blocks[i].mha.Wv, b + ".mha.Wv");
            load_param(model->transformer_blocks[i].mha.Wo, b + ".mha.Wo");
            load_param(model->transformer_blocks[i].ffn.W1, b + ".ffn.W1");
            load_param(model->transformer_blocks[i].ffn.W2, b + ".ffn.W2");
        }
        is_model_loaded = true;
    } catch (...) { is_model_loaded = false; }
    return is_model_loaded;
}

bool ctisslm::save_model(const std::string& path) {
    if (!model) return false;
    TissDB::Json::JsonObject root, weights, config;
    auto save_param = [&](const Parameter& p, const std::string& name) {
        TissDB::Json::JsonArray arr;
        for (auto v : p.value) arr.push_back(v);
        weights[name] = arr;
    };
    save_param(model->embeddings, "embeddings");
    save_param(model->output_proj, "output_proj");
    for (size_t i = 0; i < model->transformer_blocks.size(); ++i) {
        std::string b = "blk." + std::to_string(i);
        save_param(model->transformer_blocks[i].mha.Wq, b + ".mha.Wq");
        save_param(model->transformer_blocks[i].mha.Wk, b + ".mha.Wk");
        save_param(model->transformer_blocks[i].mha.Wv, b + ".mha.Wv");
        save_param(model->transformer_blocks[i].mha.Wo, b + ".mha.Wo");
        save_param(model->transformer_blocks[i].ffn.W1, b + ".ffn.W1");
        save_param(model->transformer_blocks[i].ffn.W2, b + ".ffn.W2");
    }
    config["vocab_size"] = (double)model->embeddings.shape[0];
    config["n_embd"] = (double)model->embeddings.shape[1];
    config["n_layer"] = (double)model->transformer_blocks.size();
    root["config"] = config; root["weights"] = weights;
    std::ofstream ofs(path);
    if (ofs.is_open()) { ofs << TissDB::Json::JsonValue(root).serialize(); ofs.close(); return true; }
    return false;
}

bool ctisslm::load_tokenizer(const std::string& tokenizer_path) {
    if (tokenizer_instance) {
        tokenizer_instance->bpe_tokenizer.load(tokenizer_path);
        is_tokenizer_loaded = true;
        return true;
    }
    return false;
}

std::string ctisslm::generate(const std::string& prompt, const GenerationConfig& config) {
    if (!is_model_loaded || !is_tokenizer_loaded) return "Error: Model or Tokenizer not loaded.";
    std::vector<int> tokens = tokenizer_instance->tokenize(prompt);
    for (int i = 0; i < config.max_length; ++i) {
        std::vector<double> logits = model->forward(tokens);
        size_t vocab_size = model->output_proj.shape[1];
        size_t last_token_start = (tokens.size() - 1) * vocab_size;
        int next_token = 0;
        double max_logit = -1e9;
        for (size_t v = 0; v < vocab_size; ++v) {
            if (logits[last_token_start + v] > max_logit) { max_logit = logits[last_token_start + v]; next_token = v; }
        }
        tokens.push_back(next_token);
        if (next_token == 0) break;
    }
    return tokenizer_instance->detokenize(tokens);
}

}

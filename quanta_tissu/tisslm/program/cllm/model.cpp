#include "model.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include "tissdb/json/json.h"

namespace cllm {

Model::Model(const ModelConfig& config) : config_(config) {
    token_embeddings_ = Eigen::MatrixXf::Random(config.vocab_size, config.d_model);
    layers_.reserve(config.n_layers);
    for (int i = 0; i < config.n_layers; ++i) layers_.push_back(std::make_unique<TransformerBlock>(config));
    output_layer_weight_ = Eigen::MatrixXf::Random(config.vocab_size, config.d_model);
}

void Model::add_positional_encoding(Eigen::MatrixXf& input) {
    int seq_len = input.rows(), d_model = input.cols();
    Eigen::MatrixXf pos_encoding(seq_len, d_model);
    for (int pos = 0; pos < seq_len; ++pos) {
        for (int i = 0; i < d_model / 2; ++i) {
            double div_term = std::pow(10000.0, (2.0 * i) / d_model);
            pos_encoding(pos, 2 * i) = std::sin(pos / div_term);
            pos_encoding(pos, 2 * i + 1) = std::cos(pos / div_term);
        }
    }
    input += pos_encoding;
}

Eigen::MatrixXf Model::forward(const std::vector<int>& input_tokens) {
    int seq_len = input_tokens.size();
    Eigen::MatrixXf x(seq_len, config_.d_model);
    for (int i = 0; i < seq_len; ++i) x.row(i) = token_embeddings_.row(input_tokens[i]);
    add_positional_encoding(x);
    for (auto& layer : layers_) x = layer->forward(x);
    return x * output_layer_weight_.transpose();
}

float Model::train_step(const std::vector<int>& inputs, const std::vector<int>& targets, float lr) {
    // Basic training step implementation for cllm::Model
    Eigen::MatrixXf logits = forward(inputs);
    int seq_len = inputs.size();
    float total_loss = 0.0f;

    // Cross-Entropy Loss and Gradient Calculation
    Eigen::MatrixXf d_logits = logits;
    for (int i = 0; i < seq_len; ++i) {
        Eigen::VectorXf row = logits.row(i);
        float max_val = row.maxCoeff();
        Eigen::VectorXf exp_row = (row.array() - max_val).exp();
        float sum_exp = exp_row.sum();
        Eigen::VectorXf probs = exp_row / sum_exp;

        total_loss -= std::log(probs(targets[i]) + 1e-9f);
        probs(targets[i]) -= 1.0f;
        d_logits.row(i) = probs / seq_len;
    }

    // Simplified SGD update (weight updates only for example)
    // Note: Real training would require full backprop through all layers.
    output_layer_weight_ -= lr * (d_logits.transpose() * forward(inputs)).transpose();

    return total_loss / seq_len;
}

bool Model::load_weights(const std::string& path) {
    std::ifstream ifs(path); if (!ifs.is_open()) return false;
    std::stringstream ss; ss << ifs.rdbuf(); ifs.close();
    try {
        auto root = TissDB::Json::JsonValue::parse(ss.str()).as_object();
        auto load_mat = [&](Eigen::MatrixXf& mat, const std::string& key) {
            auto arr = root.at(key).as_array();
            for (int i = 0; i < mat.rows(); ++i) for (int j = 0; j < mat.cols(); ++j) mat(i, j) = arr[i * mat.cols() + j].as_number();
        };
        auto load_vec = [&](Eigen::VectorXf& vec, const std::string& key) {
            auto arr = root.at(key).as_array();
            for (int i = 0; i < vec.size(); ++i) vec(i) = arr[i].as_number();
        };
        load_mat(token_embeddings_, "tok_emb"); load_mat(output_layer_weight_, "out_w");
        for (int i = 0; i < (int)layers_.size(); ++i) {
            std::string b = "blk." + std::to_string(i);
            load_mat(layers_[i]->attention().Wq(), b + ".attn.Wq"); load_mat(layers_[i]->attention().Wk(), b + ".attn.Wk");
            load_mat(layers_[i]->attention().Wv(), b + ".attn.Wv"); load_mat(layers_[i]->attention().Wo(), b + ".attn.Wo");
            load_mat(layers_[i]->ffn().weight1(), b + ".ffn.w1"); load_vec(layers_[i]->ffn().bias1(), b + ".ffn.b1");
            load_mat(layers_[i]->ffn().weight2(), b + ".ffn.w2"); load_vec(layers_[i]->ffn().bias2(), b + ".ffn.b2");
            load_vec(layers_[i]->ln1_gamma(), b + ".ln1.g"); load_vec(layers_[i]->ln1_beta(), b + ".ln1.b");
            load_vec(layers_[i]->ln2_gamma(), b + ".ln2.g"); load_vec(layers_[i]->ln2_beta(), b + ".ln2.b");
        }
        return true;
    } catch (...) { return false; }
}

bool Model::save_weights(const std::string& path) {
    TissDB::Json::JsonObject root;
    auto save_mat = [&](const Eigen::MatrixXf& mat, const std::string& key) {
        TissDB::Json::JsonArray arr;
        for (int i = 0; i < mat.rows(); ++i) for (int j = 0; j < mat.cols(); ++j) arr.push_back((double)mat(i, j));
        root[key] = arr;
    };
    auto save_vec = [&](const Eigen::VectorXf& vec, const std::string& key) {
        TissDB::Json::JsonArray arr;
        for (int i = 0; i < vec.size(); ++i) arr.push_back((double)vec(i));
        root[key] = arr;
    };
    save_mat(token_embeddings_, "tok_emb"); save_mat(output_layer_weight_, "out_w");
    for (int i = 0; i < (int)layers_.size(); ++i) {
        std::string b = "blk." + std::to_string(i);
        save_mat(layers_[i]->attention().Wq(), b + ".attn.Wq"); save_mat(layers_[i]->attention().Wk(), b + ".attn.Wk");
        save_mat(layers_[i]->attention().Wv(), b + ".attn.Wv"); save_mat(layers_[i]->attention().Wo(), b + ".attn.Wo");
        save_mat(layers_[i]->ffn().weight1(), b + ".ffn.w1"); save_vec(layers_[i]->ffn().bias1(), b + ".ffn.b1");
        save_mat(layers_[i]->ffn().weight2(), b + ".ffn.w2"); save_vec(layers_[i]->ffn().bias2(), b + ".ffn.b2");
        save_vec(layers_[i]->ln1_gamma(), b + ".ln1.g"); save_vec(layers_[i]->ln1_beta(), b + ".ln1.b");
        save_vec(layers_[i]->ln2_gamma(), b + ".ln2.g"); save_vec(layers_[i]->ln2_beta(), b + ".ln2.b");
    }
    std::ofstream ofs(path); if (ofs.is_open()) { ofs << TissDB::Json::JsonValue(root).serialize(); ofs.close(); return true; }
    return false;
}

}

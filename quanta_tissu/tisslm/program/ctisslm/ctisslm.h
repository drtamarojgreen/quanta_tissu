#ifndef QUANTA_TISSU_CTISSLM_H
#define QUANTA_TISSU_CTISSLM_H

#include <string>
#include <vector>
#include <map>
#include "layers.h"
#include "parameter.h"

namespace quanta_tissu {

/**
 * @brief Positional Encoding module.
 * Adds relative position information to input embeddings.
 */
class PositionalEncoding {
public:
    std::vector<double> pe;

    PositionalEncoding(size_t d_model, size_t max_len = 5000) {
        pe.resize(max_len * d_model);
        for (size_t pos = 0; pos < max_len; ++pos) {
            for (size_t i = 0; i < d_model; ++i) {
                if (i % 2 == 0) pe[pos * d_model + i] = std::sin(pos / std::pow(10000.0, static_cast<double>(i) / d_model));
                else pe[pos * d_model + i] = std::cos(pos / std::pow(10000.0, static_cast<double>(i - 1) / d_model));
            }
        }
    }

    std::vector<double> forward(const std::vector<double>& x, size_t start_pos = 0) {
        size_t d_model = pe.size() / 5000;
        size_t seq_len = x.size() / d_model;
        std::vector<double> out = x;
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = 0; j < d_model; ++j) out[i * d_model + j] += pe[(start_pos + i) * d_model + j];
        }
        return out;
    }

    std::vector<double> backward(const std::vector<double>& d_out) { return d_out; }
};

/**
 * @brief Individual Transformer Block.
 */
class TransformerBlock {
public:
    MultiHeadAttention mha;
    FeedForward ffn;
    LayerNorm ln1, ln2;
    std::string name;

    TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, const std::string& block_name = "")
        : mha(d_model, num_heads, block_name + ".mha"), ffn(d_model, d_ff, block_name + ".ffn"),
          ln1(d_model, 1e-6, block_name + ".ln1"), ln2(d_model, 1e-6, block_name + ".ln2"),
          name(block_name) {}

    std::vector<double> forward(const std::vector<double>& x, const std::vector<double>& mask = {}, const std::vector<double>& kv_cache = {}) {
        std::vector<double> x_norm1 = ln1.forward(x);
        std::vector<double> attn_out = mha.forward(x_norm1, mask, kv_cache);
        std::vector<double> x_resid1(x.size());
        for (size_t i = 0; i < x.size(); ++i) x_resid1[i] = x[i] + attn_out[i];
        std::vector<double> x_norm2 = ln2.forward(x_resid1);
        std::vector<double> ffn_out = ffn.forward(x_norm2);
        std::vector<double> out(x.size());
        for (size_t i = 0; i < x.size(); ++i) out[i] = x_resid1[i] + ffn_out[i];
        return out;
    }

    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        // Simple sequential backward assuming cache stores intermediates
        std::vector<double> d_resid2 = d_out;
        std::vector<double> d_ffn = ffn.backward(d_resid2, cache);
        std::vector<double> d_ln2 = ln2.backward(d_ffn, cache);
        std::vector<double> d_resid1(d_out.size());
        for (size_t i = 0; i < d_out.size(); ++i) d_resid1[i] = d_out[i] + d_ln2[i];
        std::vector<double> d_mha = mha.backward(d_resid1, cache);
        std::vector<double> d_ln1 = ln1.backward(d_mha, cache);
        std::vector<double> dx(d_out.size());
        for (size_t i = 0; i < d_out.size(); ++i) dx[i] = d_resid1[i] + d_ln1[i];
        return dx;
    }

    std::vector<Parameter*> parameters() {
        std::vector<Parameter*> p;
        for (auto x : mha.parameters()) p.push_back(x);
        for (auto x : ffn.parameters()) p.push_back(x);
        for (auto x : ln1.parameters()) p.push_back(x);
        for (auto x : ln2.parameters()) p.push_back(x);
        return p;
    }
};

/**
 * @brief QuantaTissu Model architecture.
 */
class QuantaTissuModel {
public:
    Parameter embeddings;
    PositionalEncoding pos_encoding;
    std::vector<TransformerBlock> transformer_blocks;
    Parameter output_proj;

    QuantaTissuModel(const std::map<std::string, int>& config)
        : embeddings({static_cast<size_t>(config.at("vocab_size")), static_cast<size_t>(config.at("n_embd"))}, "embeddings"),
          pos_encoding(static_cast<size_t>(config.at("n_embd"))),
          output_proj({static_cast<size_t>(config.at("n_embd")), static_cast<size_t>(config.at("vocab_size"))}, "output_proj")
    {
        for (int i = 0; i < config.at("n_layer"); ++i) {
            transformer_blocks.emplace_back(static_cast<size_t>(config.at("n_embd")), static_cast<size_t>(config.at("n_head")), static_cast<size_t>(config.at("d_ff")), "transformer_blocks." + std::to_string(i));
        }
    }

    std::vector<double> forward(const std::vector<int>& token_ids, const std::vector<double>& kv_cache = {}, size_t start_pos = 0) {
        size_t seq_len = token_ids.size(), d_model = embeddings.shape[1];
        std::vector<double> x(seq_len * d_model);
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = 0; j < d_model; ++j) x[i * d_model + j] = embeddings(token_ids[i], j);
        }
        x = pos_encoding.forward(x, start_pos);
        for (auto& block : transformer_blocks) x = block.forward(x, {}, kv_cache);
        size_t vocab_size = output_proj.shape[1];
        std::vector<double> logits(seq_len * vocab_size, 0.0);
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t v = 0; v < vocab_size; ++v) {
                for (size_t j = 0; j < d_model; ++j) logits[i * vocab_size + v] += x[i * d_model + j] * output_proj(j, v);
            }
        }
        return logits;
    }

    void backward(const std::vector<double>& d_logits, const std::vector<double>& cache_x, const std::vector<int>& token_ids) {
        size_t seq_len = token_ids.size(), d_model = embeddings.shape[1], vocab_size = output_proj.shape[1];
        std::vector<double> dx(seq_len * d_model, 0.0);
        // 1. Output proj backward
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t v = 0; v < vocab_size; ++v) {
                for (size_t j = 0; j < d_model; ++j) {
                    output_proj.grad[j * vocab_size + v] += cache_x[i * d_model + j] * d_logits[i * vocab_size + v];
                    dx[i * d_model + j] += d_logits[i * vocab_size + v] * output_proj.value[j * vocab_size + v];
                }
            }
        }
        // 2. Transformer blocks backward
        for (int i = transformer_blocks.size() - 1; i >= 0; --i) dx = transformer_blocks[i].backward(dx, cache_x);
        // 3. Positional encoding and Embedding backward
        dx = pos_encoding.backward(dx);
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = 0; j < d_model; ++j) embeddings.grad[token_ids[i] * d_model + j] += dx[i * d_model + j];
        }
    }

    std::vector<Parameter*> parameters() {
        std::vector<Parameter*> p;
        p.push_back(&embeddings); p.push_back(&output_proj);
        for (auto& b : transformer_blocks) for (auto param : b.parameters()) p.push_back(param);
        return p;
    }
};

struct GenerationConfig { int max_length = 100; float temperature = 1.0f; int top_k = 50; };

class ctisslm {
public:
    ctisslm(); ~ctisslm();
    bool load_model(const std::string& path);
    bool save_model(const std::string& path);
    bool load_tokenizer(const std::string& path);
    std::string generate(const std::string& prompt, const GenerationConfig& config);
private:
    bool is_model_loaded, is_tokenizer_loaded;
    QuantaTissuModel* model;
    void* tokenizer_instance;
};

}

#endif // QUANTA_TISSU_CTISSLM_H

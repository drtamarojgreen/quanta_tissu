#ifndef TRANSFORMERBLOCK_H
#define TRANSFORMERBLOCK_H

#include "multiheadattention.h"
#include "feedforward.h"
#include "layernorm.h"

class TransformerBlock {
public:
    TransformerBlock(int d_model, int num_heads, int d_ff);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);
    Matrix backward(const Matrix& d_out, const Matrix& cache_x);

    size_t get_parameter_count() const {
        return mha.get_parameter_count() + ffn.get_parameter_count() +
               ln1.get_parameter_count() + ln2.get_parameter_count();
    }

    std::vector<Matrix*> get_parameters() {
        std::vector<Matrix*> p;
        auto mha_p = mha.get_parameters(); p.insert(p.end(), mha_p.begin(), mha_p.end());
        auto ffn_p = ffn.get_parameters(); p.insert(p.end(), ffn_p.begin(), ffn_p.end());
        p.push_back(&ln1.gamma); p.push_back(&ln1.beta);
        p.push_back(&ln2.gamma); p.push_back(&ln2.beta);
        return p;
    }

    std::vector<Matrix*> get_gradients() {
        std::vector<Matrix*> g;
        auto mha_g = mha.get_gradients(); g.insert(g.end(), mha_g.begin(), mha_g.end());
        auto ffn_g = ffn.get_gradients(); g.insert(g.end(), ffn_g.begin(), ffn_g.end());
        g.push_back(&ln1.d_gamma); g.push_back(&ln1.d_beta);
        g.push_back(&ln2.d_gamma); g.push_back(&ln2.d_beta);
        return g;
    }

private:
    MultiHeadAttention mha;
    FeedForward ffn;
    LayerNorm ln1;
    LayerNorm ln2;
};

#endif // TRANSFORMERBLOCK_H

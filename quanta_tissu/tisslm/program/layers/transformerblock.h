#ifndef TRANSFORMERBLOCK_H
#define TRANSFORMERBLOCK_H

#include "multiheadattention.h"
#include "feedforward.h"
#include "layernorm.h"

class TransformerBlock {
public:
    TransformerBlock(int d_model, int num_heads, int d_ff);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);
    size_t get_parameter_count() const {
        return mha.get_parameter_count() + ffn.get_parameter_count() +
               ln1.get_parameter_count() + ln2.get_parameter_count();
    }

private:
    MultiHeadAttention mha;
    FeedForward ffn;
    LayerNorm ln1;
    LayerNorm ln2;
};

#endif // TRANSFORMERBLOCK_H
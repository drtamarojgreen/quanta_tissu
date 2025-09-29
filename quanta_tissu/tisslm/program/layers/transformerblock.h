#ifndef TRANSFORMERBLOCK_H
#define TRANSFORMERBLOCK_H

#include "multiheadattention.h"
#include "feedforward.h"
#include "layernorm.h"

class TransformerBlock {
public:
    TransformerBlock(int d_model, int num_heads, int d_ff);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);

private:
    MultiHeadAttention mha;
    FeedForward ffn;
    LayerNorm ln1;
    LayerNorm ln2;
};

#endif // TRANSFORMERBLOCK_H
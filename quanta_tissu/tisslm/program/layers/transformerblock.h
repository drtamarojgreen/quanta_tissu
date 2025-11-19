#ifndef TRANSFORMERBLOCK_H
#define TRANSFORMERBLOCK_H

#include "configurableattention.h"
#include "feedforward.h"
#include "layernorm.h"

class TransformerBlock {
public:
    TransformerBlock(int d_model, int num_heads, int d_ff, AttentionMode attention_mode);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);

private:
    ConfigurableAttention attention;
    FeedForward ffn;
    LayerNorm ln1;
    LayerNorm ln2;
};

#endif // TRANSFORMERBLOCK_H
#ifndef MODEL_H
#define MODEL_H

#include "../layers/transformerblock.h"
#include "../layers/matrix.h"
#include <vector>

class Model {
public:
    Model(int vocab_size, int d_model, int n_layer, int n_head, int d_ff, AttentionMode attention_mode);

    Matrix forward(const std::vector<int>& token_ids);

private:
    Matrix embeddings;
    Matrix positional_encoding;
    std::vector<TransformerBlock> transformer_blocks;
    Matrix output_proj;

    Matrix create_positional_encoding(int max_len, int d_model);
};

#endif // MODEL_H
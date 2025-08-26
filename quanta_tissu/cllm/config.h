#ifndef CLLM_CONFIG_H
#define CLLM_CONFIG_H

#include <cstddef>

namespace cllm {

// Configuration for the Transformer model
struct ModelConfig {
    int d_model = 512;          // The dimension of the model's embeddings and hidden states
    int n_heads = 8;            // The number of attention heads in the multi-head attention layers
    int n_layers = 6;           // The number of transformer blocks (encoder/decoder layers)
    int vocab_size = 32000;     // The size of the vocabulary
    int max_seq_len = 512;      // The maximum sequence length the model can handle
    double dropout = 0.1;       // The dropout rate for regularization
};

} // namespace cllm

#endif // CLLM_CONFIG_H

#ifndef CLLM_ATTENTION_H
#define CLLM_ATTENTION_H

#include "config.h"
#include <Eigen/Dense>

namespace cllm {

class MultiHeadAttention {
public:
    explicit MultiHeadAttention(const ModelConfig& config);

    // Performs the forward pass for the multi-head attention layer.
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);

private:
    // Helper function for scaled dot-product attention
    Eigen::MatrixXf scaled_dot_product_attention(const Eigen::MatrixXf& q, const Eigen::MatrixXf& k, const Eigen::MatrixXf& v);

    ModelConfig config_;

    // Weight matrices for Q, K, V projections
    Eigen::MatrixXf Wq_;
    Eigen::MatrixXf Wk_;
    Eigen::MatrixXf Wv_;

    // Weight matrix for the output projection
    Eigen::MatrixXf Wo_;
};

} // namespace cllm

#endif // CLLM_ATTENTION_H

#ifndef CLLM_ATTENTION_H
#define CLLM_ATTENTION_H

#include "config.h"
#include <Eigen/Dense>

namespace cllm {

class MultiHeadAttention {
public:
    explicit MultiHeadAttention(const ModelConfig& config);
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);

    Eigen::MatrixXf& Wq() { return Wq_; }
    Eigen::MatrixXf& Wk() { return Wk_; }
    Eigen::MatrixXf& Wv() { return Wv_; }
    Eigen::MatrixXf& Wo() { return Wo_; }

private:
    Eigen::MatrixXf scaled_dot_product_attention(const Eigen::MatrixXf& q, const Eigen::MatrixXf& k, const Eigen::MatrixXf& v);
    ModelConfig config_;
    Eigen::MatrixXf Wq_, Wk_, Wv_, Wo_;
};

}

#endif // CLLM_ATTENTION_H

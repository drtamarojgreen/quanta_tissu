#ifndef CLLM_FEED_FORWARD_H
#define CLLM_FEED_FORWARD_H

#include "config.h"
#include <Eigen/Dense>

namespace cllm {

class FeedForward {
public:
    explicit FeedForward(const ModelConfig& config);
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);
    Eigen::MatrixXf backward(const Eigen::MatrixXf& d_out, const Eigen::MatrixXf& x, float lr);

    Eigen::MatrixXf& weight1() { return weight1_; }
    Eigen::VectorXf& bias1() { return bias1_; }
    Eigen::MatrixXf& weight2() { return weight2_; }
    Eigen::VectorXf& bias2() { return bias2_; }

private:
    ModelConfig config_;
    int hidden_dim;
    Eigen::MatrixXf weight1_;
    Eigen::VectorXf bias1_;
    Eigen::MatrixXf weight2_;
    Eigen::VectorXf bias2_;
};

}

#endif // CLLM_FEED_FORWARD_H

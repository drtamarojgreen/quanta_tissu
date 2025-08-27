#ifndef CLLM_FEED_FORWARD_H
#define CLLM_FEED_FORWARD_H

#include "config.h"
#include <Eigen/Dense>

namespace cllm {

class FeedForward {
public:
    explicit FeedForward(const ModelConfig& config);

    // Performs the forward pass for the feed-forward network.
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);

private:
    ModelConfig config_;

    // Parameters for the two linear layers.
    // The inner dimension is usually larger than d_model.
    int hidden_dim;
    Eigen::MatrixXf weight1_;
    Eigen::VectorXf bias1_;
    Eigen::MatrixXf weight2_;
    Eigen::VectorXf bias2_;
};

} // namespace cllm

#endif // CLLM_FEED_FORWARD_H

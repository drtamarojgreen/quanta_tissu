#include "feed_forward.h"
#include <iostream>

namespace cllm {

FeedForward::FeedForward(const ModelConfig& config) : config_(config) {
    // A common choice for the hidden dimension is 4 * d_model.
    hidden_dim = 4 * config.d_model;

    // Initialize weights and biases.
    // In a real implementation, these would be loaded from a checkpoint.
    // For now, we'll initialize them with random values.
    weight1_ = Eigen::MatrixXf::Random(hidden_dim, config.d_model);
    bias1_ = Eigen::VectorXf::Random(hidden_dim);
    weight2_ = Eigen::MatrixXf::Random(config.d_model, hidden_dim);
    bias2_ = Eigen::VectorXf::Random(config.d_model);
}

// Performs the forward pass for the feed-forward network.
Eigen::MatrixXf FeedForward::forward(const Eigen::MatrixXf& input) {
    // First linear transformation
    Eigen::MatrixXf hidden = (input * weight1_.transpose()).rowwise() + bias1_.transpose();

    // ReLU activation function (element-wise)
    hidden = hidden.cwiseMax(0);

    // Second linear transformation
    Eigen::MatrixXf output = (hidden * weight2_.transpose()).rowwise() + bias2_.transpose();

    return output;
}

Eigen::MatrixXf FeedForward::backward(const Eigen::MatrixXf& d_out, const Eigen::MatrixXf& x, float lr) {
    Eigen::MatrixXf z = (x * weight1_.transpose()).rowwise() + bias1_.transpose();
    Eigen::MatrixXf h = z.cwiseMax(0);

    Eigen::MatrixXf d_weight2 = d_out.transpose() * h;
    Eigen::VectorXf d_bias2 = d_out.colwise().sum();
    Eigen::MatrixXf d_h = d_out * weight2_;

    Eigen::MatrixXf d_z = (z.array() > 0).cast<float>().matrix().cwiseProduct(d_h);
    Eigen::MatrixXf d_weight1 = d_z.transpose() * x;
    Eigen::VectorXf d_bias1 = d_z.colwise().sum();

    weight1_ -= lr * d_weight1;
    bias1_ -= lr * d_bias1;
    weight2_ -= lr * d_weight2;
    bias2_ -= lr * d_bias2;

    return d_z * weight1_;
}

} // namespace cllm

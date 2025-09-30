#include "loss_function.h"
#include <algorithm>
#include <limits>

namespace TissDB {
namespace TissLM {
namespace Core {

Matrix CrossEntropyLoss::softmax(const Matrix& input) {
    Matrix output = input;
    for (int r = 0; r < output.rows(); ++r) {
        float max_val = -std::numeric_limits<float>::infinity();
        for (int c = 0; c < output.cols(); ++c) {
            if (output.get(r, c) > max_val) {
                max_val = output.get(r, c);
            }
        }

        float sum_exp = 0.0f;
        for (int c = 0; c < output.cols(); ++c) {
            output.set(r, c, std::exp(output.get(r, c) - max_val));
            sum_exp += output.get(r, c);
        }

        for (int c = 0; c < output.cols(); ++c) {
            output.set(r, c, output.get(r, c) / sum_exp);
        }
    }
    return output;
}

float CrossEntropyLoss::compute_loss(const Matrix& predictions, const Matrix& targets) {
    // Ensure predictions and targets have the same dimensions
    if (predictions.rows() != targets.rows() || predictions.cols() != targets.cols()) {
        throw std::runtime_error("Predictions and targets must have the same dimensions for CrossEntropyLoss.");
    }

    Matrix softmax_predictions = softmax(predictions);
    float loss = 0.0f;
    int num_samples = predictions.rows();

    for (int r = 0; r < num_samples; ++r) {
        for (int c = 0; c < predictions.cols(); ++c) {
            // Only sum if target is 1 (one-hot encoding)
            if (targets.get(r, c) > 0.5f) { // Assuming targets are 0 or 1
                loss -= targets.get(r, c) * std::log(softmax_predictions.get(r, c) + std::numeric_limits<float>::epsilon());
            }
        }
    }
    return loss / num_samples;
}

Matrix CrossEntropyLoss::compute_gradient(const Matrix& predictions, const Matrix& targets) {
    // Ensure predictions and targets have the same dimensions
    if (predictions.rows() != targets.rows() || predictions.cols() != targets.cols()) {
        throw std::runtime_error("Predictions and targets must have the same dimensions for CrossEntropyLoss gradient.");
    }

    Matrix softmax_predictions = softmax(predictions);
    // Gradient of Cross-Entropy loss after softmax is (softmax_predictions - targets)
    return softmax_predictions - targets;
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

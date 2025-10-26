#include "loss_function.h"
#include <algorithm>
#include <limits>

namespace TissLM {
namespace Training {

using namespace TissNum;

Matrix CrossEntropyLoss::softmax(const Matrix& input) {
    Matrix output = input;
    for (int r = 0; r < output.rows(); ++r) {
        float max_val = -std::numeric_limits<float>::infinity();
        for (int c = 0; c < output.cols(); ++c) {
            if (output(r, c) > max_val) {
                max_val = output(r, c);
            }
        }

        float sum_exp = 0.0f;
        for (int c = 0; c < output.cols(); ++c) {
            output(r, c) = std::exp(output(r, c) - max_val);
            sum_exp += output(r, c);
        }

        for (int c = 0; c < output.cols(); ++c) {
            output(r, c) = output(r, c) / sum_exp;
        }
    }
    return output;
}

float CrossEntropyLoss::compute_loss(const Matrix& predictions, const Matrix& targets) {
    Matrix softmax_predictions = softmax(predictions);
    float loss = 0.0f;
    int num_samples = predictions.rows();

    for (int r = 0; r < num_samples; ++r) {
        int target_class = static_cast<int>(targets(r, 0));
        loss -= std::log(softmax_predictions(r, target_class) + std::numeric_limits<float>::epsilon());
    }
    return loss / num_samples;
}

Matrix CrossEntropyLoss::compute_gradient(const Matrix& predictions, const Matrix& targets) {
    Matrix softmax_predictions = softmax(predictions);
    for (int i = 0; i < predictions.rows(); ++i) {
        int target_class = static_cast<int>(targets(i, 0));
        softmax_predictions(i, target_class) -= 1.0f;
    }
    return softmax_predictions / predictions.rows();
}

} // namespace Training
} // namespace TissLM

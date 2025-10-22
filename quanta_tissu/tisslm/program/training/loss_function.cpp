#include "loss_function.h"
#include <algorithm>
#include <limits>

namespace TissDB {
namespace TissLM {
namespace Core {

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
        int target_class = -1;
        for (int c = 0; c < targets.cols(); ++c) {
            if (targets(r, c) == 1.0f) {
                target_class = c;
                break;
            }
        }
        if (target_class != -1) {
            loss -= std::log(softmax_predictions(r, target_class) + 1e-9);
        }
    }
    return loss / num_samples;
}

Matrix CrossEntropyLoss::compute_gradient(const Matrix& predictions, const Matrix& targets) {
    Matrix softmax_predictions = softmax(predictions);
    Matrix gradient = softmax_predictions;

    for (int r = 0; r < predictions.rows(); ++r) {
        int target_class = -1;
        for (int c = 0; c < targets.cols(); ++c) {
            if (targets(r, c) == 1.0f) {
                target_class = c;
                break;
            }
        }
        if (target_class != -1) {
            gradient(r, target_class) -= 1.0f;
        }
    }
    return gradient / predictions.rows();
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

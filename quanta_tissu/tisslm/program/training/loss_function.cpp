#include "loss_function.h"
#include <algorithm>
#include <limits>
#include <vector>

namespace TissLM {
namespace Training {

using namespace TissNum;

float CrossEntropyLoss::compute_loss(const Matrix& predictions, const Matrix& targets) {
    Matrix softmax_predictions = Matrix::softmax(predictions);
    float loss = 0.0f;
    int num_samples = predictions.shape()[0];

    for (int r = 0; r < num_samples; ++r) {
        int target_class = static_cast<int>(targets.at({r, 0}));
        loss -= std::log(softmax_predictions.at({r, target_class}) + std::numeric_limits<float>::epsilon());
    }
    return loss / num_samples;
}

Matrix CrossEntropyLoss::compute_gradient(const Matrix& predictions, const Matrix& targets) {
    Matrix softmax_predictions = Matrix::softmax(predictions);
    for (int i = 0; i < predictions.shape()[0]; ++i) {
        int target_class = static_cast<int>(targets.at({i, 0}));
        softmax_predictions.at({i, target_class}) -= 1.0f;
    }
    return softmax_predictions / predictions.shape()[0];
}

} // namespace Training
} // namespace TissLM

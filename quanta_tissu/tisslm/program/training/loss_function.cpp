#include "loss_function.h"
#include "tests/model/analyzer/error_handler.hpp"
#include <algorithm>
#include <limits>
#include <cmath>

namespace TissLM {
namespace Training {

using namespace TissNum;

Matrix CrossEntropyLoss::softmax(const Matrix& input) {
    Matrix output = input;
    for (size_t r = 0; r < output.rows(); ++r) {
        float max_val = -std::numeric_limits<float>::infinity();
        for (size_t c = 0; c < output.cols(); ++c) if (output({r, c}) > max_val) max_val = output({r, c});
        float sum_exp = 0.0f;
        for (size_t c = 0; c < output.cols(); ++c) {
            output({r, c}) = std::exp(output({r, c}) - max_val);
            sum_exp += output({r, c});
        }
        if (sum_exp == 0.0f) {
            RMA_ERROR_VAL(rma::ErrorType::FLOAT_PRECISION, sum_exp, "Softmax sum_exp is zero");
        }
        for (size_t c = 0; c < output.cols(); ++c) output({r, c}) /= sum_exp;
    }
    return output;
}

float CrossEntropyLoss::compute_loss(const Matrix& predictions, const Matrix& targets) {
    Matrix sm = softmax(predictions);
    float loss = 0.0f;
    for (size_t r = 0; r < predictions.rows(); ++r) {
        int t = static_cast<int>(targets({r, 0}));
        loss -= std::log(sm({r, (size_t)t}) + 1e-9f);
    }
    float final_loss = loss / predictions.rows();
    if (std::isnan(final_loss) || std::isinf(final_loss)) {
        RMA_ERROR_VAL(rma::ErrorType::FLOAT_PRECISION, (double)final_loss, "NaN or Inf loss detected");
    }
    return final_loss;
}

Matrix CrossEntropyLoss::compute_gradient(const Matrix& predictions, const Matrix& targets) {
    Matrix sm = softmax(predictions);
    for (size_t i = 0; i < predictions.rows(); ++i) sm({i, (size_t)targets({i, 0})}) -= 1.0f;
    return sm / (float)predictions.rows();
}

}
}

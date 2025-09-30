#include "optimizer.h"
#include <cmath>

namespace TissDB {
namespace TissLM {
namespace Core {

Adam::Adam(float learning_rate, float beta1, float beta2, float epsilon)
    : learning_rate_(learning_rate), beta1_(beta1), beta2_(beta2), epsilon_(epsilon) {
}

void Adam::update(std::vector<std::shared_ptr<Parameter>>& parameters) {
    t_++;

    float bias_correction1 = 1.0f - std::pow(beta1_, t_);
    float bias_correction2 = 1.0f - std::pow(beta2_, t_);

    if (m_.empty()) {
        m_.resize(parameters.size());
        v_.resize(parameters.size());
        for (size_t i = 0; i < parameters.size(); ++i) {
            m_[i] = Matrix(parameters[i]->value.rows(), parameters[i]->value.cols(), 0.0f);
            v_[i] = Matrix(parameters[i]->value.rows(), parameters[i]->value.cols(), 0.0f);
        }
    }

    for (size_t i = 0; i < parameters.size(); ++i) {
        if (!parameters[i]->grad.has_value()) {
            // Skip parameters that don't have gradients
            continue;
        }
        const Matrix& grad = parameters[i]->grad.value();

        // Update biased first moment estimate
        m_[i] = (m_[i] * beta1_) + (grad * (1.0f - beta1_));

        // Update biased second raw moment estimate
        v_[i] = (v_[i] * beta2_) + (grad.element_wise_product(grad) * (1.0f - beta2_));

        // Compute bias-corrected first moment estimate
        Matrix m_hat = m_[i] / bias_correction1;

        // Compute bias-corrected second raw moment estimate
        Matrix v_hat = v_[i] / bias_correction2;

        // Update parameters
        parameters[i]->value = parameters[i]->value - (m_hat * learning_rate_).element_wise_division(v_hat.element_wise_sqrt() + epsilon_);
    }
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

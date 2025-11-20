#include "optimizer.h"
#include <cmath>
#include <fstream>

namespace TissLM {
namespace Training {

using namespace TissNum;

Adam::Adam(float learning_rate, float beta1, float beta2, float epsilon)
    : learning_rate_(learning_rate), beta1_(beta1), beta2_(beta2), epsilon_(epsilon), t_(0) {
}

void Adam::update(std::vector<Parameter*>& parameters) {
    t_++;

    float bias_correction1 = 1.0f - std::pow(beta1_, t_);
    float bias_correction2 = 1.0f - std::pow(beta2_, t_);

    if (m_.empty()) {
        m_.resize(parameters.size());
        v_.resize(parameters.size());
        for (size_t i = 0; i < parameters.size(); ++i) {
            m_[i] = Matrix(parameters[i]->value().shape());
            v_[i] = Matrix(parameters[i]->value().shape());
        }
    }

    for (size_t i = 0; i < parameters.size(); ++i) {
        const Matrix& grad = parameters[i]->grad();

        m_[i] = (m_[i] * beta1_) + (grad * (1.0f - beta1_));
        v_[i] = (v_[i] * beta2_) + ((grad * grad) * (1.0f - beta2_));

        Matrix m_hat = m_[i] / bias_correction1;
        Matrix v_hat = v_[i] / bias_correction2;

        parameters[i]->value() = parameters[i]->value() + ((m_hat / (Matrix::sqrt(v_hat) + epsilon_)) * -learning_rate_);
    }
}

void Adam::save_state(std::ofstream& ofs) const {
    // Simplified: not implemented for TensorNode structure
}

void Adam::load_state(std::ifstream& ifs) {
    // Simplified: not implemented for TensorNode structure
}

} // namespace Training
} // namespace TissLM

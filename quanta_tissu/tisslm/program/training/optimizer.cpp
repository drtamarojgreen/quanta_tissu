#include "optimizer.h"
#include <cmath>
#include <fstream>

namespace TissLM {
namespace Training {

using namespace TissNum;

Adam::Adam(float learning_rate, float beta1, float beta2, float epsilon)
    : learning_rate_(learning_rate), beta1_(beta1), beta2_(beta2), epsilon_(epsilon) {
}

void Adam::update(std::vector<Parameter*>& parameters) {
    t_++;

    float bias_correction1 = 1.0f - std::pow(beta1_, t_);
    float bias_correction2 = 1.0f - std::pow(beta2_, t_);

    if (m_.empty()) {
        m_.resize(parameters.size());
        v_.resize(parameters.size());
        for (size_t i = 0; i < parameters.size(); ++i) {
            m_[i] = Matrix::zeros({parameters[i]->value().rows(), parameters[i]->value().cols()});
            v_[i] = Matrix::zeros({parameters[i]->value().rows(), parameters[i]->value().cols()});
        }
    }

    for (size_t i = 0; i < parameters.size(); ++i) {
        // Gradients are always present for parameters in this model
        const Matrix& grad = parameters[i]->grad();

        // Update biased first moment estimate
        m_[i] = (m_[i] * beta1_) + (grad * (1.0f - beta1_));

        // Update biased second raw moment estimate
        v_[i] = (v_[i] * beta2_) + (grad.element_wise_product(grad) * (1.0f - beta2_));

        // Compute bias-corrected first moment estimate
        Matrix m_hat = m_[i] / bias_correction1;

        // Compute bias-corrected second raw moment estimate
        Matrix v_hat = v_[i] / bias_correction2;

        // Update parameters
        parameters[i]->value() = parameters[i]->value() - (m_hat * learning_rate_).element_wise_division(v_hat.element_wise_sqrt() + epsilon_);
    }
}

void Adam::save_state(std::ofstream& ofs) const {
    ofs.write(reinterpret_cast<const char*>(&t_), sizeof(t_));
    size_t m_size = m_.size();
    ofs.write(reinterpret_cast<const char*>(&m_size), sizeof(m_size));
    for (const auto& matrix : m_) {
        auto shape = matrix.get_shape();
        size_t shape_size = shape.size();
        ofs.write(reinterpret_cast<const char*>(&shape_size), sizeof(shape_size));
        ofs.write(reinterpret_cast<const char*>(shape.data()), shape_size * sizeof(size_t));
        size_t data_size = matrix.data_size();
        ofs.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
        ofs.write(reinterpret_cast<const char*>(matrix.get_data()), data_size * sizeof(float));
    }
    size_t v_size = v_.size();
    ofs.write(reinterpret_cast<const char*>(&v_size), sizeof(v_size));
    for (const auto& matrix : v_) {
        auto shape = matrix.get_shape();
        size_t shape_size = shape.size();
        ofs.write(reinterpret_cast<const char*>(&shape_size), sizeof(shape_size));
        ofs.write(reinterpret_cast<const char*>(shape.data()), shape_size * sizeof(size_t));
        size_t data_size = matrix.data_size();
        ofs.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
        ofs.write(reinterpret_cast<const char*>(matrix.get_data()), data_size * sizeof(float));
    }
}

void Adam::load_state(std::ifstream& ifs) {
    ifs.read(reinterpret_cast<char*>(&t_), sizeof(t_));
    size_t m_size;
    ifs.read(reinterpret_cast<char*>(&m_size), sizeof(m_size));
    m_.resize(m_size);
    for (size_t i = 0; i < m_size; ++i) {
        size_t shape_size;
        ifs.read(reinterpret_cast<char*>(&shape_size), sizeof(shape_size));
        std::vector<size_t> shape(shape_size);
        ifs.read(reinterpret_cast<char*>(shape.data()), shape_size * sizeof(size_t));
        m_[i] = TissNum::Matrix(shape);
        size_t data_size;
        ifs.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        ifs.read(reinterpret_cast<char*>(m_[i].get_data()), data_size * sizeof(float));
    }
    size_t v_size;
    ifs.read(reinterpret_cast<char*>(&v_size), sizeof(v_size));
    v_.resize(v_size);
    for (size_t i = 0; i < v_size; ++i) {
        size_t shape_size;
        ifs.read(reinterpret_cast<char*>(&shape_size), sizeof(shape_size));
        std::vector<size_t> shape(shape_size);
        ifs.read(reinterpret_cast<char*>(shape.data()), shape_size * sizeof(size_t));
        v_[i] = TissNum::Matrix(shape);
        size_t data_size;
        ifs.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        ifs.read(reinterpret_cast<char*>(v_[i].get_data()), data_size * sizeof(float));
    }
}

} // namespace Training
} // namespace TissLM

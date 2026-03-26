#include "optimizer.h"
#include <cmath>
#include <fstream>

namespace TissLM {
namespace Training {

using namespace TissNum;

Adam::Adam(float learning_rate, float beta1, float beta2, float epsilon)
    : learning_rate_(learning_rate), beta1_(beta1), beta2_(beta2), epsilon_(epsilon) {}

void Adam::update(std::vector<Parameter*>& parameters) {
    t_++;
    float bc1 = 1.0f - std::pow(beta1_, t_);
    float bc2 = 1.0f - std::pow(beta2_, t_);
    if (m_.empty()) {
        m_.resize(parameters.size()); v_.resize(parameters.size());
        for (size_t i = 0; i < parameters.size(); ++i) {
            m_[i] = Matrix::zeros(parameters[i]->value().get_shape());
            v_[i] = Matrix::zeros(parameters[i]->value().get_shape());
        }
    }
    for (size_t i = 0; i < parameters.size(); ++i) {
        const Matrix& grad = parameters[i]->grad();
        m_[i] = (m_[i] * beta1_) + (grad * (1.0f - beta1_));
        v_[i] = (v_[i] * beta2_) + (grad.element_wise_product(grad) * (1.0f - beta2_));
        Matrix mh = m_[i] / bc1;
        Matrix vh = v_[i] / bc2;
        parameters[i]->value() = parameters[i]->value() - (mh * learning_rate_).element_wise_division(vh.element_wise_sqrt() + epsilon_);
    }
}

void Adam::save_state(std::ofstream& ofs) const {
    ofs.write(reinterpret_cast<const char*>(&t_), sizeof(t_));
    size_t sz = m_.size(); ofs.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
    for (const auto& m : m_) {
        auto sh = m.get_shape(); size_t shs = sh.size();
        ofs.write(reinterpret_cast<const char*>(&shs), sizeof(shs));
        ofs.write(reinterpret_cast<const char*>(sh.data()), shs * sizeof(size_t));
        size_t ds = m.data_size(); ofs.write(reinterpret_cast<const char*>(&ds), sizeof(ds));
        ofs.write(reinterpret_cast<const char*>(m.get_data()), ds * sizeof(float));
    }
    sz = v_.size(); ofs.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
    for (const auto& m : v_) {
        auto sh = m.get_shape(); size_t shs = sh.size();
        ofs.write(reinterpret_cast<const char*>(&shs), sizeof(shs));
        ofs.write(reinterpret_cast<const char*>(sh.data()), shs * sizeof(size_t));
        size_t ds = m.data_size(); ofs.write(reinterpret_cast<const char*>(&ds), sizeof(ds));
        ofs.write(reinterpret_cast<const char*>(m.get_data()), ds * sizeof(float));
    }
}

void Adam::load_state(std::ifstream& ifs) {
    ifs.read(reinterpret_cast<char*>(&t_), sizeof(t_));
    size_t sz; ifs.read(reinterpret_cast<char*>(&sz), sizeof(sz)); m_.resize(sz);
    for (size_t i = 0; i < sz; ++i) {
        size_t shs; ifs.read(reinterpret_cast<char*>(&shs), sizeof(shs));
        std::vector<size_t> sh(shs); ifs.read(reinterpret_cast<char*>(sh.data()), shs * sizeof(size_t));
        m_[i] = TissNum::Matrix(sh);
        size_t ds; ifs.read(reinterpret_cast<char*>(&ds), sizeof(ds));
        ifs.read(reinterpret_cast<char*>(const_cast<float*>(m_[i].get_data())), ds * sizeof(float));
    }
    ifs.read(reinterpret_cast<char*>(&sz), sizeof(sz)); v_.resize(sz);
    for (size_t i = 0; i < sz; ++i) {
        size_t shs; ifs.read(reinterpret_cast<char*>(&shs), sizeof(shs));
        std::vector<size_t> sh(shs); ifs.read(reinterpret_cast<char*>(sh.data()), shs * sizeof(size_t));
        v_[i] = TissNum::Matrix(sh);
        size_t ds; ifs.read(reinterpret_cast<char*>(&ds), sizeof(ds));
        ifs.read(reinterpret_cast<char*>(const_cast<float*>(v_[i].get_data())), ds * sizeof(float));
    }
}

}
}

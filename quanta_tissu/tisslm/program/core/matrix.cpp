#include "matrix.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <functional>

namespace TissNum {

Matrix::Matrix(const std::vector<size_t>& shape) : shape_(shape) {
    size_t total_size = 1;
    for (size_t dim : shape) {
        total_size *= dim;
    }
    data_.resize(total_size, 0.0f);
}

float& Matrix::operator()(const std::vector<size_t>& indices) {
    size_t index = 0;
    size_t stride = 1;
    for (int i = shape_.size() - 1; i >= 0; --i) {
        index += indices[i] * stride;
        stride *= shape_[i];
    }
    return data_[index];
}

const float& Matrix::operator()(const std::vector<size_t>& indices) const {
    size_t index = 0;
    size_t stride = 1;
    for (int i = shape_.size() - 1; i >= 0; --i) {
        index += indices[i] * stride;
        stride *= shape_[i];
    }
    return data_[index];
}

Matrix Matrix::random(const std::vector<size_t>& shape) {
    Matrix m(shape);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        m.data_[i] = dist(gen);
    }
    return m;
}

Matrix Matrix::zeros(const std::vector<size_t>& shape) {
    return Matrix(shape);
}

Matrix Matrix::ones(const std::vector<size_t>& shape) {
    Matrix m(shape);
    std::fill(m.data_.begin(), m.data_.end(), 1.0f);
    return m;
}

Matrix Matrix::transpose() const {
    if (shape_.size() != 2) {
        throw std::invalid_argument("Default transpose is only supported for 2D matrices.");
    }
    return transpose(0, 1);
}

Matrix Matrix::reshape(const std::vector<size_t>& new_shape) const {
    size_t total_size = 1;
    for (size_t dim : new_shape) {
        total_size *= dim;
    }
    if (total_size != data_.size()) {
        throw std::invalid_argument("Total size of new shape must match old shape.");
    }
    Matrix result(new_shape);
    result.data_ = data_;
    return result;
}

Matrix Matrix::transpose(int dim1, int dim2) const {
    if (dim1 >= shape_.size() || dim2 >= shape_.size()) {
        throw std::out_of_range("Invalid dimensions for transpose.");
    }
    std::vector<size_t> new_shape = shape_;
    std::swap(new_shape[dim1], new_shape[dim2]);
    Matrix result(new_shape);

    std::vector<size_t> old_indices(shape_.size());
    std::function<void(int)> recurse = 
        [&](int k) {
        if (k == shape_.size()) {
            std::vector<size_t> new_indices = old_indices;
            std::swap(new_indices[dim1], new_indices[dim2]);
            result(new_indices) = (*this)(old_indices);
            return;
        }
        for (size_t i = 0; i < shape_[k]; ++i) {
            old_indices[k] = i;
            recurse(k + 1);
        }
    };

    recurse(0);
    return result;
}

Matrix Matrix::operator+(const Matrix& other) const {
    return broadcast_op(other, std::plus<float>());
}

Matrix Matrix::operator-(const Matrix& other) const {
    return broadcast_op(other, std::minus<float>());
}

Matrix Matrix::operator*(const Matrix& other) const {
    return broadcast_op(other, std::multiplies<float>());
}

Matrix Matrix::operator/(const Matrix& other) const {
    return broadcast_op(other, std::divides<float>());
}

Matrix Matrix::matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() == 2 && b.get_shape().size() == 2) {
        if (a.cols() != b.rows()) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 2D multiplication.");
        }
        Matrix result({a.rows(), b.cols()});
        for (size_t i = 0; i < a.rows(); ++i) {
            for (size_t j = 0; j < b.cols(); ++j) {
                for (size_t k = 0; k < a.cols(); ++k) {
                    result({i, j}) += a({i, k}) * b({k, j});
                }
            }
        }
        return result;
    }

    if (a.get_shape().size() == 3 && b.get_shape().size() == 2) {
        if (a.get_shape()[2] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 3D x 2D multiplication.");
        }
        size_t batch_size = a.get_shape()[0];
        size_t seq_len = a.get_shape()[1];
        size_t dim = a.get_shape()[2];
        size_t new_dim = b.get_shape()[1];

        Matrix result({batch_size, seq_len, new_dim});
        for (size_t i = 0; i < batch_size; ++i) {
            for (size_t j = 0; j < seq_len; ++j) {
                for (size_t k = 0; k < new_dim; ++k) {
                    float sum = 0.0f;
                    for (size_t l = 0; l < dim; ++l) {
                        sum += a({i, j, l}) * b({l, k});
                    }
                    result({i, j, k}) = sum;
                }
            }
        }
        return result;
    }

    if (a.get_shape().size() == 1 && b.get_shape().size() == 2) {
        if (a.get_shape()[0] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 1D x 2D multiplication.");
        }
        size_t dim = a.get_shape()[0];
        size_t new_dim = b.get_shape()[1];

        Matrix result({new_dim});
        for (size_t i = 0; i < new_dim; ++i) {
            float sum = 0.0f;
            for (size_t j = 0; j < dim; ++j) {
                sum += a({j}) * b({j, i});
            }
            result({i}) = sum;
        }
        return result;
    }

    if (a.get_shape().size() == 2 && b.get_shape().size() == 1) {
        if (a.get_shape()[1] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 2D x 1D multiplication.");
        }
        size_t rows = a.get_shape()[0];
        size_t cols = a.get_shape()[1];

        Matrix result({rows});
        for (size_t i = 0; i < rows; ++i) {
            float sum = 0.0f;
            for (size_t j = 0; j < cols; ++j) {
                sum += a({i, j}) * b({j});
            }
            result({i}) = sum;
        }
        return result;
    }

    throw std::invalid_argument("matmul not implemented for these matrix shapes.");
}

Matrix Matrix::sum(int axis) const {
    if (axis == -1) {
        Matrix result({1});
        result.data_[0] = std::accumulate(data_.begin(), data_.end(), 0.0f);
        return result;
    }

    std::vector<size_t> new_shape = shape_;
    new_shape[axis] = 1;
    Matrix result(new_shape);
    result.data_.assign(result.data_.size(), 0.0f);

    std::vector<size_t> indices(shape_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
        size_t temp_i = i;
        for (int d = shape_.size() - 1; d >= 0; --d) {
            indices[d] = temp_i % shape_[d];
            temp_i /= shape_[d];
        }

        std::vector<size_t> new_indices = indices;
        new_indices[axis] = 0;
        result(new_indices) += data_[i];
    }
    return result;
}

Matrix Matrix::mean(int axis) const {
    Matrix s = sum(axis);
    if (axis == -1) {
        return s / data_.size();
    }
    return s / shape_[axis];
}

Matrix Matrix::variance(int axis) const {
    Matrix m = mean(axis);
    Matrix diff = (*this) - m;
    Matrix sq_diff = diff * diff;
    return sq_diff.mean(axis);
}

Matrix Matrix::max(int axis) const {
    if (axis == -1) {
        Matrix result({1});
        result.data_[0] = *std::max_element(data_.begin(), data_.end());
        return result;
    }

    std::vector<size_t> new_shape = shape_;
    new_shape[axis] = 1;
    Matrix result(new_shape);
    result.data_.assign(result.data_.size(), -std::numeric_limits<float>::infinity());

    std::vector<size_t> indices(shape_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
        size_t temp_i = i;
        for (int d = shape_.size() - 1; d >= 0; --d) {
            indices[d] = temp_i % shape_[d];
            temp_i /= shape_[d];
        }

        std::vector<size_t> new_indices = indices;
        new_indices[axis] = 0;
        if (data_[i] > result(new_indices)) {
            result(new_indices) = data_[i];
        }
    }
    return result;
}

Matrix Matrix::sqrt(const Matrix& m) {
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::sqrt(m.data_[i]);
    }
    return result;
}

Matrix Matrix::pow(const Matrix& m, float exponent) {
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::pow(m.data_[i], exponent);
    }
    return result;
}

Matrix Matrix::exp(const Matrix& m) {
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::exp(m.data_[i]);
    }
    return result;
}

Matrix Matrix::operator+(float scalar) const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + scalar;
    }
    return result;
}

Matrix Matrix::operator-(float scalar) const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - scalar;
    }
    return result;
}

Matrix Matrix::operator*(float scalar) const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * scalar;
    }
    return result;
}

Matrix Matrix::operator/(float scalar) const {
    if (scalar == 0.0f) {
        throw std::invalid_argument("Division by zero");
    }
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] / scalar;
    }
    return result;
}

Matrix operator/(float scalar, const Matrix& m) {
    if (scalar == 0.0f) {
        throw std::invalid_argument("Division by zero");
    }
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = scalar / m.data_[i];
    }
    return result;
}

Matrix Matrix::element_wise_product(const Matrix& other) const {
    return (*this) * other;
}

Matrix Matrix::element_wise_division(const Matrix& other) const {
    return (*this) / other;
}

Matrix Matrix::element_wise_sqrt() const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        if (data_[i] < 0.0f) {
            throw std::invalid_argument("Cannot take square root of negative number in element_wise_sqrt.");
        }
        result.data_[i] = std::sqrt(data_[i]);
    }
    return result;
}

Matrix Matrix::concatenate(const Matrix& a, const Matrix& b, int axis) {
    if (a.shape_.size() != b.shape_.size()) {
        throw std::invalid_argument("Matrices must have the same number of dimensions to concatenate.");
    }
    for (size_t i = 0; i < a.shape_.size(); ++i) {
        if (i != axis && a.shape_[i] != b.shape_[i]) {
            throw std::invalid_argument("Matrix dimensions must match for concatenation, except for the concatenation axis.");
        }
    }

    std::vector<size_t> new_shape = a.shape_;
    new_shape[axis] += b.shape_[axis];
    Matrix result(new_shape);

    // This is a simplified implementation for concatenation. A full implementation would be more complex.
    // For now, we assume 2D concatenation.
    if (a.shape_.size() != 2) {
        throw std::invalid_argument("Concatenation is only supported for 2D matrices for now.");
    }

    if (axis == 0) { // Concatenate along rows
        for (size_t r = 0; r < a.rows(); ++r) {
            for (size_t c = 0; c < a.cols(); ++c) {
                result({r, c}) = a({r, c});
            }
        }
        for (size_t r = 0; r < b.rows(); ++r) {
            for (size_t c = 0; c < b.cols(); ++c) {
                result({a.rows() + r, c}) = b({r, c});
            }
        }
    } else if (axis == 1) { // Concatenate along columns
        for (size_t r = 0; r < a.rows(); ++r) {
            for (size_t c = 0; c < a.cols(); ++c) {
                result({r, c}) = a({r, c});
            }
            for (size_t c = 0; c < b.cols(); ++c) {
                result({r, a.cols() + c}) = b({r, c});
            }
        }
    }
    return result;
}

Matrix operator*(float scalar, const Matrix& m) {
    return m * scalar;
}

Matrix Matrix::batch_matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() != 4 || b.get_shape().size() != 4) {
        throw std::invalid_argument("batch_matmul only supports 4D matrices.");
    }
    if (a.get_shape()[0] != b.get_shape()[0] || a.get_shape()[1] != b.get_shape()[1]) {
        throw std::invalid_argument("Batch dimensions must match for batch_matmul.");
    }
    if (a.get_shape()[3] != b.get_shape()[2]) {
        throw std::invalid_argument("Matrix dimensions are not compatible for multiplication.");
    }

    size_t batch_size = a.get_shape()[0];
    size_t num_heads = a.get_shape()[1];
    size_t seq_len_q = a.get_shape()[2];
    size_t seq_len_k = b.get_shape()[3];
    size_t head_dim = a.get_shape()[3];

    Matrix result({batch_size, num_heads, seq_len_q, seq_len_k});

    for (size_t i = 0; i < batch_size; ++i) {
        for (size_t j = 0; j < num_heads; ++j) {
            for (size_t l = 0; l < seq_len_q; ++l) {
                for (size_t m = 0; m < seq_len_k; ++m) {
                    float sum = 0.0f;
                    for (size_t n = 0; n < head_dim; ++n) {
                        sum += a({i, j, l, n}) * b({i, j, n, m});
                    }
                    result({i, j, l, m}) = sum;
                }
            }
        }
    }
    return result;
}

} // namespace TissNum

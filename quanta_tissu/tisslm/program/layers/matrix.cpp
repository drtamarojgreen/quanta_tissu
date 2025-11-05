#include "matrix.h"
#include <stdexcept>
#include <random>
#include <numeric>
#include <functional>

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
    if (shape_ != other.shape_) {
        throw std::invalid_argument("Matrix dimensions must match for addition.");
    }
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (shape_ != other.shape_) {
        throw std::invalid_argument("Matrix dimensions must match for subtraction.");
    }
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - other.data_[i];
    }
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (shape_ != other.shape_) {
        throw std::invalid_argument("Matrix dimensions must match for element-wise multiplication.");
    }
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * other.data_[i];
    }
    return result;
}

Matrix Matrix::matmul(const Matrix& a, const Matrix& b) {
    if (a.cols() != b.rows()) {
        throw std::invalid_argument("Matrix dimensions are not compatible for multiplication.");
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

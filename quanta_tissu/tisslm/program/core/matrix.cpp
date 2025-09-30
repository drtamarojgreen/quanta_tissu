#include "matrix.h"
#include <random>
#include <algorithm>

namespace TissNum {

Matrix Matrix::random(size_t rows, size_t cols) {
    Matrix m(rows, cols);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    for (size_t i = 0; i < rows * cols; ++i) {
        m.data_[i] = dist(gen);
    }
    return m;
}

Matrix Matrix::zeros(size_t rows, size_t cols) {
    return Matrix(rows, cols);
}

Matrix Matrix::transpose() const {
    Matrix result(cols_, rows_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            result(j, i) = (*this)(i, j);
        }
    }
    return result;
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for addition.");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for subtraction.");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - other.data_[i];
    }
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for element-wise multiplication.");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * other.data_[i];
    }
    return result;
}

Matrix Matrix::operator/(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for element-wise division.");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        if (other.data_[i] == 0.0f) {
            throw std::invalid_argument("Division by zero in element-wise division.");
        }
        result.data_[i] = data_[i] / other.data_[i];
    }
    return result;
}

Matrix Matrix::matmul(const Matrix& a, const Matrix& b) {
    if (a.cols_ != b.rows_) {
        throw std::invalid_argument("Matrix dimensions are not compatible for multiplication.");
    }
    Matrix result = Matrix::zeros(a.rows_, b.cols_);
    for (size_t i = 0; i < a.rows_; ++i) {
        for (size_t j = 0; j < b.cols_; ++j) {
            for (size_t k = 0; k < a.cols_; ++k) {
                result(i, j) += a(i, k) * b(k, j);
            }
        }
    }
    return result;
}

Matrix Matrix::ones(size_t rows, size_t cols) {
    Matrix m(rows, cols);
    std::fill(m.data_.begin(), m.data_.end(), 1.0f);
    return m;
}

Matrix Matrix::mean(int axis) const {
    if (axis == 1) { // Mean across columns
        Matrix result(rows_, 1);
        for (size_t i = 0; i < rows_; ++i) {
            float sum = 0.0f;
            for (size_t j = 0; j < cols_; ++j) {
                sum += (*this)(i, j);
            }
            result(i, 0) = sum / cols_;
        }
        return result;
    } else { // Mean across rows
        Matrix result(1, cols_);
        for (size_t j = 0; j < cols_; ++j) {
            float sum = 0.0f;
            for (size_t i = 0; i < rows_; ++i) {
                sum += (*this)(i, j);
            }
            result(0, j) = sum / rows_;
        }
        return result;
    }
}

Matrix Matrix::variance(int axis, const Matrix& mean) const {
    if (axis == 1) { // Variance across columns
        Matrix result(rows_, 1);
        for (size_t i = 0; i < rows_; ++i) {
            float sum_sq_diff = 0.0f;
            for (size_t j = 0; j < cols_; ++j) {
                float diff = (*this)(i, j) - mean(i, 0);
                sum_sq_diff += diff * diff;
            }
            result(i, 0) = sum_sq_diff / cols_;
        }
        return result;
    } else { // Variance across rows
        Matrix result(1, cols_);
        for (size_t j = 0; j < cols_; ++j) {
            float sum_sq_diff = 0.0f;
            for (size_t i = 0; i < rows_; ++i) {
                float diff = (*this)(i, j) - mean(0, j);
                sum_sq_diff += diff * diff;
            }
            result(0, j) = sum_sq_diff / rows_;
        }
        return result;
    }
}

Matrix Matrix::sqrt(const Matrix& m) {
    Matrix result(m.rows_, m.cols_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::sqrt(m.data_[i]);
    }
    return result;
}

Matrix Matrix::sum(int axis) const {
    if (axis == 0) { // Sum along rows
        Matrix result(1, cols_);
        for (size_t j = 0; j < cols_; ++j) {
            float sum_val = 0.0f;
            for (size_t i = 0; i < rows_; ++i) {
                sum_val += (*this)(i, j);
            }
            result(0, j) = sum_val;
        }
        return result;
    } else if (axis == 1) { // Sum along columns
        Matrix result(rows_, 1);
        for (size_t i = 0; i < rows_; ++i) {
            float sum_val = 0.0f;
            for (size_t j = 0; j < cols_; ++j) {
                sum_val += (*this)(i, j);
            }
            result(i, 0) = sum_val;
        }
        return result;
    } else {
        throw std::invalid_argument("Invalid axis for sum operation. Must be 0 or 1.");
    }
}

// Scalar operations
Matrix Matrix::operator+(float scalar) const {
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + scalar;
    }
    return result;
}

Matrix Matrix::operator-(float scalar) const {
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - scalar;
    }
    return result;
}

Matrix Matrix::operator*(float scalar) const {
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * scalar;
    }
    return result;
}

Matrix Matrix::operator/(float scalar) const {
    if (scalar == 0.0f) {
        throw std::invalid_argument("Division by zero");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] / scalar;
    }
    return result;
}

Matrix Matrix::concatenate(const Matrix& a, const Matrix& b, int axis) {
    if (axis == 0) { // Concatenate along rows
        if (a.cols_ != b.cols_) {
            throw std::invalid_argument("Matrices must have the same number of columns to concatenate along rows.");
        }
        Matrix result(a.rows_ + b.rows_, a.cols_);
        for (size_t r = 0; r < a.rows_; ++r) {
            for (size_t c = 0; c < a.cols_; ++c) {
                result(r, c) = a(r, c);
            }
        }
        for (size_t r = 0; r < b.rows_; ++r) {
            for (size_t c = 0; c < b.cols_; ++c) {
                result(a.rows_ + r, c) = b(r, c);
            }
        }
        return result;
    } else if (axis == 1) { // Concatenate along columns
        if (a.rows_ != b.rows_) {
            throw std::invalid_argument("Matrices must have the same number of rows to concatenate along columns.");
        }
        Matrix result(a.rows_, a.cols_ + b.cols_);
        for (size_t r = 0; r < a.rows_; ++r) {
            for (size_t c = 0; c < a.cols_; ++c) {
                result(r, c) = a(r, c);
            }
            for (size_t c = 0; c < b.cols_; ++c) {
                result(r, a.cols_ + c) = b(r, c);
            }
        }
        return result;
    } else {
        throw std::invalid_argument("Invalid axis for concatenate operation. Must be 0 or 1.");
    }
}

} // namespace TissNum

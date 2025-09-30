#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

namespace TissNum {

class Matrix {
public:
    // Constructors
    Matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows * cols, 0.0f) {}
    Matrix() : rows_(0), cols_(0) {}

    // Accessors
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    float& operator()(size_t row, size_t col) {
        if (row >= rows_ || col >= cols_) {
            throw std::out_of_range("Matrix access out of range");
        }
        return data_[row * cols_ + col];
    }

    const float& operator()(size_t row, size_t col) const {
        if (row >= rows_ || col >= cols_) {
            throw std::out_of_range("Matrix access out of range");
        }
        return data_[row * cols_ + col];
    }

    // Basic matrix operations (placeholders for now)
    static Matrix random(size_t rows, size_t cols);
    static Matrix zeros(size_t rows, size_t cols);
    static Matrix ones(size_t rows, size_t cols);
    Matrix transpose() const;

    // Statistical operations
    Matrix mean(int axis) const;
    Matrix variance(int axis, const Matrix& mean) const;
    static Matrix sqrt(const Matrix& m);
    Matrix sum(int axis) const;

    // Element-wise operations
    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const; // Element-wise multiplication
    Matrix operator/(const Matrix& other) const; // Element-wise division
    
    // Scalar operations
    Matrix operator+(float scalar) const;
    Matrix operator-(float scalar) const;
    Matrix operator*(float scalar) const;
    Matrix operator/(float scalar) const;

    // Matrix multiplication
    static Matrix matmul(const Matrix& a, const Matrix& b);

    // Concatenation
    static Matrix concatenate(const Matrix& a, const Matrix& b, int axis);

private:
    size_t rows_;
    size_t cols_;
    std::vector<float> data_;
};

} // namespace TissNum

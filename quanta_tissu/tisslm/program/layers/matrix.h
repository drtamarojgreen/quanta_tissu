#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>

/**
 * @brief Indigenous Matrix class for the program architecture.
 * Refined for consistency and compatibility.
 */
class Matrix {
public:
    explicit Matrix(const std::vector<size_t>& shape);
    Matrix() : shape_({0, 0}) {}
    Matrix(size_t rows, size_t cols) : Matrix(std::vector<size_t>{rows, cols}) {}

    float& operator()(const std::vector<size_t>& indices);
    const float& operator()(const std::vector<size_t>& indices) const;
    float& operator()(size_t r, size_t c) { return (*this)({r, c}); }
    const float& operator()(size_t r, size_t c) const { return (*this)({r, c}); }

    size_t rows() const { return shape_.size() > 0 ? shape_[0] : 0; }
    size_t cols() const { return shape_.size() > 1 ? shape_[1] : 0; }
    size_t size() const { return data_.size(); }
    const std::vector<size_t>& get_shape() const { return shape_; }

    static Matrix random(const std::vector<size_t>& shape);
    static Matrix random(size_t r, size_t c) { return random(std::vector<size_t>{r, c}); }
    static Matrix zeros(const std::vector<size_t>& shape);
    static Matrix zeros(size_t r, size_t c) { return zeros(std::vector<size_t>{r, c}); }

    Matrix transpose() const;
    Matrix transpose(int dim1, int dim2) const;
    Matrix reshape(const std::vector<size_t>& new_shape) const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const;

    Matrix operator/(float scalar) const {
        Matrix result(shape_);
        for (size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] / scalar;
        return result;
    }

    static Matrix matmul(const Matrix& a, const Matrix& b);
    static Matrix transpose(const Matrix& m);
    static Matrix exp(const Matrix& m);

private:
    std::vector<size_t> shape_;
    std::vector<float> data_;
};

#endif // MATRIX_H

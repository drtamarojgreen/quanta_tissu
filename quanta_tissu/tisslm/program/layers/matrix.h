#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>

class Matrix {
public:
    Matrix(const std::vector<size_t>& shape);
    Matrix() : shape_({0, 0}) {}

    float& operator()(const std::vector<size_t>& indices);
    const float& operator()(const std::vector<size_t>& indices) const;

    size_t rows() const { return shape_.size() > 0 ? shape_[0] : 0; }
    size_t cols() const { return shape_.size() > 1 ? shape_[1] : 0; }
    const std::vector<size_t>& get_shape() const { return shape_; }

    static Matrix random(const std::vector<size_t>& shape);
    static Matrix zeros(const std::vector<size_t>& shape);

    Matrix transpose() const;
    Matrix transpose(int dim1, int dim2) const;
    Matrix reshape(const std::vector<size_t>& new_shape) const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const; // Element-wise

    // Scalar operations
    Matrix operator/(float scalar) const {
        Matrix result(shape_);
        for (size_t i = 0; i < data_.size(); ++i) {
            result.data_[i] = data_[i] / scalar;
        }
        return result;
    }

    static Matrix matmul(const Matrix& a, const Matrix& b);

private:
    std::vector<size_t> shape_;
    std::vector<float> data_;
};

#endif // MATRIX_H

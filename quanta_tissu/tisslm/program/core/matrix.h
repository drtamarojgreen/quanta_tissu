#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

namespace TissNum {

class Matrix {
public:
    // Constructors
    Matrix(const std::vector<size_t>& shape);
    Matrix() : shape_({0, 0}) {}

    // Accessors
    const std::vector<size_t>& get_shape() const { return shape_; }
    size_t rows() const { return shape_.size() > 0 ? shape_[0] : 0; }
    size_t cols() const { return shape_.size() > 1 ? shape_[1] : 0; }

    float& operator()(const std::vector<size_t>& indices);

    const float& operator()(const std::vector<size_t>& indices) const;

    // Reshape and transpose
    Matrix reshape(const std::vector<size_t>& new_shape) const;
    Matrix transpose(int dim1, int dim2) const;

    // Basic matrix operations (placeholders for now)
    static Matrix random(const std::vector<size_t>& shape);
    static Matrix zeros(const std::vector<size_t>& shape);
    static Matrix ones(const std::vector<size_t>& shape);
    Matrix transpose() const;

    // Statistical operations
    Matrix mean(int axis = -1) const;
    Matrix variance(int axis = -1) const;
    Matrix max(int axis = -1) const;
    static Matrix sqrt(const Matrix& m);
    static Matrix pow(const Matrix& m, float exponent);
    static Matrix exp(const Matrix& m);
    Matrix sum(int axis = -1) const;

    // Element-wise operations
    Matrix element_wise_product(const Matrix& other) const;
    Matrix element_wise_division(const Matrix& other) const;
    Matrix element_wise_sqrt() const;
    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const; // Element-wise multiplication
    Matrix operator/(const Matrix& other) const; // Element-wise division
    
    // Scalar operations
    Matrix operator+(float scalar) const;
    Matrix operator-(float scalar) const;
    Matrix operator*(float scalar) const;
    Matrix operator/(float scalar) const;

    // Friend functions for scalar operations
    friend Matrix operator*(float scalar, const Matrix& m);
    friend Matrix operator/(float scalar, const Matrix& m);

    // Matrix multiplication
    static Matrix matmul(const Matrix& a, const Matrix& b);

    // Concatenation
    static Matrix concatenate(const Matrix& a, const Matrix& b, int axis);

    // Raw data access
    const float* get_data() const { return data_.data(); }

private:
    std::vector<size_t> shape_;
    std::vector<float> data_;
};

} // namespace TissNum

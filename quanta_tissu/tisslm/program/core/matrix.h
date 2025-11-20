#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <functional>
#include <optional>

namespace TissNum {

class Matrix {
public:
    // Constructors
    Matrix() : data_(4, std::vector<std::optional<float>>(4, std::nullopt)) {}

    // Copy/Move Semantics
    Matrix(const Matrix&) = default;
    Matrix(Matrix&&) noexcept = default;
    Matrix& operator=(const Matrix&) = default;
    Matrix& operator=(Matrix&&) noexcept = default;

    // Accessors
    std::vector<size_t> get_shape() const {
        size_t dim0 = 0;
        size_t dim1 = 0;

        bool has_any_value = false;
        for (size_t r = 0; r < 4; ++r) {
            bool row_has_value = false;
            size_t row_last_idx = 0;
            for (size_t c = 0; c < 4; ++c) {
                if (data_[r][c].has_value()) {
                    row_has_value = true;
                    has_any_value = true;
                    row_last_idx = c;
                }
            }

            if (row_has_value) {
                dim0++;
                if (row_last_idx >= dim1) {
                    dim1 = row_last_idx + 1;
                }
            }
        }

        if (!has_any_value) {
            return {0, 0};
        }

        return {dim0, dim1};
    }
    size_t rows() const { return get_shape()[0]; }
    size_t cols() const { return get_shape()[1]; }

    float& operator()(const std::vector<size_t>& indices);

    const float& operator()(const std::vector<size_t>& indices) const;

    // Reshape and transpose
    Matrix reshape(const std::vector<size_t>& new_shape) const;
    Matrix transpose(int dim1, int dim2) const;

    // Basic matrix operations
    static Matrix random(const std::vector<size_t>& shape);
    static Matrix zeros(const std::vector<size_t>& shape);
    static Matrix ones(const std::vector<size_t>& shape);
    Matrix transpose() const;
    static Matrix matmul(const Matrix& a, const Matrix& b);
    Matrix matmul(const Matrix& other) const;
    static Matrix batch_matmul(const Matrix& a, const Matrix& b);
    Matrix batch_matmul(const Matrix& other) const;

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

    // Concatenation
    static Matrix concatenate(const Matrix& a, const Matrix& b, int axis);

    // Raw data access
    // Raw data access is no longer a simple pointer.
    // This will require significant refactoring in client code.
    std::vector<std::vector<std::optional<float>>> get_data() const { return data_; }

private:
    // A fixed 4x4 grid holding optional float values.
    std::vector<std::vector<std::optional<float>>> data_;
};

} // namespace TissNum
#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <functional>

namespace TissNum {

class Matrix {
public:
    // Constructors
    Matrix(const std::vector<size_t>& shape);
    Matrix() : shape_({0, 0}) {}

    // Copy/Move Semantics
    Matrix(const Matrix&) = default;
    Matrix(Matrix&&) noexcept = default;
    Matrix& operator=(const Matrix&) = default;
    Matrix& operator=(Matrix&&) noexcept = default;

    // Accessors
    const std::vector<size_t>& get_shape() const { return shape_; }
    size_t rows() const { return shape_.size() > 0 ? shape_[0] : 0; }
    size_t cols() const { return shape_.size() > 1 ? shape_[1] : 0; }

    float& operator()(const std::vector<size_t>& indices);

    const float& operator()(const std::vector<size_t>& indices) const;

    // Reshape and transpose
    Matrix reshape(const std::vector<size_t>& new_shape) const;
    Matrix transpose(int dim1, int dim2) const;

    // Basic matrix operations
    static Matrix random(const std::vector<size_t>& shape, float mean = 0.0f, float stddev = 1.0f);
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
    const float* get_data() const { return data_.data(); }
    float* get_data() { return data_.data(); }
    size_t data_size() const { return data_.size(); }

    Matrix repeat(int n, int axis) const;

private:
    std::vector<size_t> shape_;
    std::vector<float> data_;

    inline void assert_size_match(size_t old_size, size_t new_size) const {
        if (old_size != new_size) {
            throw std::runtime_error("Total size of new shape must match old shape.");
        }
    }

    static std::vector<size_t> get_broadcasted_shape(const std::vector<size_t>& shape1, const std::vector<size_t>& shape2) {
        const auto& s1 = shape1.size() > shape2.size() ? shape1 : shape2;
        const auto& s2 = shape1.size() > shape2.size() ? shape2 : shape1;
        std::vector<size_t> result_shape(s1.size());
        long diff = s1.size() - s2.size();

        for (size_t i = 0; i < s1.size(); ++i) {
            size_t dim1 = s1[i];
            size_t dim2 = (i >= diff) ? s2[i - diff] : 1;
            if (dim1 != dim2 && dim1 != 1 && dim2 != 1) {
                throw std::invalid_argument("Matrix dimensions are not compatible for broadcasting.");
            }
            result_shape[i] = std::max(dim1, dim2);
        }
        return result_shape;
    }

    template<typename Func>
    Matrix broadcast_op(const Matrix& other, Func op) const {
        std::vector<size_t> result_shape = get_broadcasted_shape(shape_, other.shape_);
        Matrix result(result_shape);

        std::vector<size_t> this_strides(shape_.size());
        if (!shape_.empty()) {
            this_strides.back() = 1;
            for (int i = shape_.size() - 2; i >= 0; --i) {
                this_strides[i] = this_strides[i + 1] * shape_[i + 1];
            }
        }

        std::vector<size_t> other_strides(other.shape_.size());
        if (!other.shape_.empty()) {
            other_strides.back() = 1;
            for (int i = other.shape_.size() - 2; i >= 0; --i) {
                other_strides[i] = other_strides[i + 1] * other.shape_[i + 1];
            }
        }

        std::vector<size_t> result_strides(result_shape.size());
        if (!result_shape.empty()) {
            result_strides.back() = 1;
            for (int i = result_shape.size() - 2; i >= 0; --i) {
                result_strides[i] = result_strides[i + 1] * result_shape[i + 1];
            }
        }

        long this_dim_offset = result_shape.size() - shape_.size();
        long other_dim_offset = result_shape.size() - other.shape_.size();

        std::function<void(int, size_t, size_t, size_t)> recurse_broadcast = 
            [&](int dim, size_t current_this_idx, size_t current_other_idx, size_t current_result_idx) {
            if (dim == result_shape.size()) {
                result.data_[current_result_idx] = op(data_[current_this_idx], other.data_[current_other_idx]);
                return;
            }

            for (size_t i = 0; i < result_shape[dim]; ++i) {
                size_t next_this_idx = current_this_idx;
                size_t next_other_idx = current_other_idx;
                size_t next_result_idx = current_result_idx + i * result_strides[dim];

                if (dim >= this_dim_offset) {
                    size_t this_dim = dim - this_dim_offset;
                    if (shape_[this_dim] != 1) {
                        next_this_idx += i * this_strides[this_dim];
                    }
                }
                if (dim >= other_dim_offset) {
                    size_t other_dim = dim - other_dim_offset;
                    if (other.shape_[other_dim] != 1) {
                        next_other_idx += i * other_strides[other_dim];
                    }
                }
                recurse_broadcast(dim + 1, next_this_idx, next_other_idx, next_result_idx);
            }
        };
        recurse_broadcast(0, 0, 0, 0);
        return result;
    }
};

} // namespace TissNum
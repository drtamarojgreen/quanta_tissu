#pragma once

#include <vector>
#include <iostream>
#include <variant>
#include <numeric>
#include <stdexcept>
#include <functional>
#include <string>

namespace TissNum {

// Forward declaration for the recursive variant
struct TensorNode;
using TensorData = std::variant<float, std::vector<TensorNode>>;

struct TensorNode {
    TensorData data;
};

class Matrix {
public:
    // Constructors & Assignment
    Matrix() = default;
    Matrix(const std::vector<int>& shape);

    // Shape and data access
    const std::vector<int>& shape() const { return shape_; }
    float& at(const std::vector<int>& indices);
    const float& at(const std::vector<int>& indices) const;

    // Reshape and transpose
    void reshape(const std::vector<int>& new_shape);
    Matrix transpose(int dim1, int dim2) const;

    // Element-wise and scalar operations
    Matrix operator+(const Matrix& other) const;
    Matrix& operator+=(const Matrix& other);
    Matrix operator+(float scalar) const;
    Matrix operator*(const Matrix& other) const; // Element-wise
    Matrix operator/(float scalar) const;

    // Statistical operations
    Matrix mean(int axis) const;
    Matrix variance(int axis) const;
    Matrix sum(int axis) const;

    // Static operations
    static Matrix random(const std::vector<int>& shape);
    static Matrix ones(const std::vector<int>& shape);
    static Matrix matmul(const Matrix& a, const Matrix& b);
    static Matrix softmax(const Matrix& m, int axis = -1);
    static Matrix sqrt(const Matrix& m);


private:
    std::vector<int> shape_;
    std::vector<TensorNode> root_;

    // Private constructor for internal use
    Matrix(const std::vector<int>& shape, bool init_with_val, float val);

    // Recursive helpers
    void build_tensor(TensorNode& node, const std::vector<int>& shape, size_t dim_idx, bool random_init, bool init_with_val, float val);
    void flatten(const TensorNode& node, std::vector<float>& out) const;
    TensorNode* get_node(const std::vector<int>& indices);
    const TensorNode* get_node(const std::vector<int>& indices) const;

    // Recursive helpers for operations
    void add_recursive(const TensorNode& a, const TensorNode& b, TensorNode& result) const;
    void add_assign_recursive(const TensorNode& other, TensorNode& self);
    void add_scalar_recursive(const TensorNode& a, float scalar, TensorNode& result) const;
    void mul_recursive(const TensorNode& a, const TensorNode& b, TensorNode& result) const;
    void div_recursive(const TensorNode& a, float scalar, TensorNode& result) const;
    void sqrt_recursive(const TensorNode& a, TensorNode& result) const;

};

} // namespace TissNum
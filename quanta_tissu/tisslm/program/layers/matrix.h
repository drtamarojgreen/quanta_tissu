#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <variant>
#include <numeric>

// Forward declaration of TensorNode for the variant
struct TensorNode;

// Define the variant that can hold either a float or a vector of TensorNodes
using TensorData = std::variant<float, std::vector<TensorNode>>;

// The actual TensorNode struct
struct TensorNode {
    TensorData data;
};

class Matrix {
public:
    // Constructors
    Matrix() = default;
    Matrix(const std::vector<int>& shape);

    // Shape and data access
    std::vector<int> shape() const;
    float& at(const std::vector<int>& indices);
    const float& at(const std::vector<int>& indices) const;

    // Operations
    void reshape(const std::vector<int>& new_shape);
    Matrix operator+(const Matrix& other) const;
    Matrix operator/(float scalar) const;
    Matrix transpose(int dim1, int dim2) const;


    // Static operations
    static Matrix random(const std::vector<int>& shape);
    static Matrix matmul(const Matrix& a, const Matrix& b);
    static Matrix softmax(const Matrix& m, int axis = -1);

    // Utility
    void print() const;

private:
    std::vector<TensorNode> root_;
    std::vector<int> shape_;

    // Private constructor for operations
    Matrix(const std::vector<int>& shape, bool internal);

    // Recursive helper functions
    void build_tensor(TensorNode& node, const std::vector<int>& shape, size_t dim_idx, bool random_init);
    void flatten(const TensorNode& node, std::vector<float>& out) const;
    const TensorNode* get_node(const std::vector<int>& indices) const;
    TensorNode* get_node(const std::vector<int>& indices);
    void print_recursive(const TensorNode& node, std::string prefix) const;

    // Helpers for operations
    void add_recursive(const TensorNode& a, const TensorNode& b, TensorNode& result) const;
    void div_recursive(const TensorNode& a, float scalar, TensorNode& result) const;
};

#endif // MATRIX_H
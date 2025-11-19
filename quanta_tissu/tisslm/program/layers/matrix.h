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

    // Utility
    void print() const;

private:
    std::vector<TensorNode> root_;
    std::vector<int> shape_;

    // Recursive helper functions
    void build_tensor(TensorNode& node, const std::vector<int>& shape, size_t dim_idx);
    void flatten(const TensorNode& node, std::vector<float>& out) const;
    const TensorNode* get_node(const std::vector<int>& indices) const;
    TensorNode* get_node(const std::vector<int>& indices);
    void print_recursive(const TensorNode& node, std::string prefix) const;
    void compute_shape_recursive(const TensorNode& node, std::vector<int>& shape) const;
};

#endif // MATRIX_H
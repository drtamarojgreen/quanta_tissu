#include "matrix.h"
#include <stdexcept>
#include <iostream>
#include <numeric>

// --- Helper Functions ---

void Matrix::build_tensor(TensorNode& node, const std::vector<int>& shape, size_t dim_idx) {
    if (dim_idx == shape.size() - 1) {
        // Last dimension, create leaf nodes (floats)
        std::vector<TensorNode> leaf_nodes(shape[dim_idx]);
        for (int i = 0; i < shape[dim_idx]; ++i) {
            leaf_nodes[i].data = 0.0f; // Initialize with zero
        }
        node.data = std::move(leaf_nodes);
    } else {
        // Create branch nodes
        std::vector<TensorNode> branch_nodes(shape[dim_idx]);
        for (int i = 0; i < shape[dim_idx]; ++i) {
            build_tensor(branch_nodes[i], shape, dim_idx + 1);
        }
        node.data = std::move(branch_nodes);
    }
}

TensorNode* Matrix::get_node(const std::vector<int>& indices) {
    if (indices.size() != shape_.size()) {
        throw std::invalid_argument("Incorrect number of indices provided.");
    }
    std::vector<TensorNode>* current_level = &root_;
    for (size_t i = 0; i < indices.size() -1; ++i) {
        int index = indices[i];
        if (index < 0 || index >= current_level->size()) {
             throw std::out_of_range("Index out of range.");
        }
        TensorNode& node = (*current_level)[index];
        current_level = std::get_if<std::vector<TensorNode>>(&node.data);
         if (!current_level) {
            throw std::runtime_error("Invalid tensor structure.");
        }
    }
    int last_index = indices.back();
    if(last_index < 0 || last_index >= current_level->size()){
         throw std::out_of_range("Index out of range.");
    }
    return &(*current_level)[last_index];
}


const TensorNode* Matrix::get_node(const std::vector<int>& indices) const {
     if (indices.size() != shape_.size()) {
        throw std::invalid_argument("Incorrect number of indices provided.");
    }
    const std::vector<TensorNode>* current_level = &root_;
    for (size_t i = 0; i < indices.size() -1; ++i) {
        int index = indices[i];
        if (index < 0 || index >= current_level->size()) {
             throw std::out_of_range("Index out of range.");
        }
        const TensorNode& node = (*current_level)[index];
        current_level = std::get_if<std::vector<TensorNode>>(&node.data);
         if (!current_level) {
            throw std::runtime_error("Invalid tensor structure.");
        }
    }
     int last_index = indices.back();
    if(last_index < 0 || last_index >= current_level->size()){
         throw std::out_of_range("Index out of range.");
    }
    return &(*current_level)[last_index];
}


void Matrix::flatten(const TensorNode& node, std::vector<float>& out) const {
    if (const float* val = std::get_if<float>(&node.data)) {
        out.push_back(*val);
    } else if (const std::vector<TensorNode>* vec = std::get_if<std::vector<TensorNode>>(&node.data)) {
        for (const auto& child_node : *vec) {
            flatten(child_node, out);
        }
    }
}


// --- Constructors ---

Matrix::Matrix(const std::vector<int>& shape) : shape_(shape) {
    if (shape.empty()) return;
    root_.resize(shape[0]);
    for(int i = 0; i < shape[0]; ++i){
        build_tensor(root_[i], shape, 1);
    }
}


// --- Public Methods ---

std::vector<int> Matrix::shape() const {
    return shape_;
}

float& Matrix::at(const std::vector<int>& indices) {
    TensorNode* node = get_node(indices);
    float* val = std::get_if<float>(&node->data);
    if (!val) {
        throw std::runtime_error("Element is not a float value.");
    }
    return *val;
}

const float& Matrix::at(const std::vector<int>& indices) const {
    const TensorNode* node = get_node(indices);
    const float* val = std::get_if<const float>(&node->data);
     if (!val) {
        throw std::runtime_error("Element is not a float value.");
    }
    return *val;
}

void Matrix::reshape(const std::vector<int>& new_shape) {
    // 1. Flatten the entire data structure
    std::vector<float> flat_data;
    for(const auto& node : root_){
        flatten(node, flat_data);
    }

    // 2. Check if the new shape is compatible
    long long new_size = 1;
    for (int dim : new_shape) {
        new_size *= dim;
    }
    if (new_size != flat_data.size()) {
        throw std::invalid_argument("New shape size is incompatible with the number of elements.");
    }

    // 3. Rebuild the tensor with the new shape
    shape_ = new_shape;
    root_.clear();
    root_.resize(shape_[0]);
     for(int i = 0; i < shape_[0]; ++i){
        build_tensor(root_[i], shape_, 1);
    }

    // 4. Repopulate the new structure with the flattened data
    int flat_idx = 0;
    std::function<void(TensorNode&)> repopulate =
        [&](TensorNode& node) {
        if (std::holds_alternative<float>(node.data)) {
            node.data = flat_data[flat_idx++];
        } else {
            for (auto& child : std::get<std::vector<TensorNode>>(node.data)) {
                repopulate(child);
            }
        }
    };

    for(auto& node : root_){
         repopulate(node);
    }
}

void Matrix::print() const {
    // This is a simplified print for debugging. A full implementation would need
    // to handle multi-dimensional formatting.
    std::vector<float> flat_data;
    for(const auto& node : root_){
        flatten(node, flat_data);
    }
    for(float val : flat_data){
        std::cout << val << " ";
    }
    std::cout << std::endl;
}
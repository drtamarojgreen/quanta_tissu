#include "matrix.h"
#include <random>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <numeric>

namespace TissNum {

// --- Helper Functions ---

void Matrix::build_tensor(TensorNode& node, const std::vector<int>& shape, size_t dim_idx, bool random_init, bool init_with_val, float val) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);

    if (dim_idx == shape.size() - 1) {
        std::vector<TensorNode> leaf_nodes(shape[dim_idx]);
        for (int i = 0; i < shape[dim_idx]; ++i) {
            if (random_init) leaf_nodes[i].data = dist(gen);
            else if (init_with_val) leaf_nodes[i].data = val;
            else leaf_nodes[i].data = 0.0f;
        }
        node.data = std::move(leaf_nodes);
    } else {
        std::vector<TensorNode> branch_nodes(shape[dim_idx]);
        for (int i = 0; i < shape[dim_idx]; ++i) {
            build_tensor(branch_nodes[i], shape, dim_idx + 1, random_init, init_with_val, val);
        }
        node.data = std::move(branch_nodes);
    }
}

TensorNode* Matrix::get_node(const std::vector<int>& indices) {
    if (indices.size() != shape_.size()) throw std::invalid_argument("Incorrect number of indices.");
    TensorNode* current_node = &root_[indices[0]];
    for (size_t i = 1; i < indices.size(); ++i) {
        std::vector<TensorNode>* vec = std::get_if<std::vector<TensorNode>>(&current_node->data);
        if (!vec || indices[i] >= (int)vec->size()) throw std::out_of_range("Index out of range.");
        current_node = &(*vec)[indices[i]];
    }
    return current_node;
}

const TensorNode* Matrix::get_node(const std::vector<int>& indices) const {
    if (indices.size() != shape_.size()) throw std::invalid_argument("Incorrect number of indices.");
    const TensorNode* current_node = &root_[indices[0]];
    for (size_t i = 1; i < indices.size(); ++i) {
        const std::vector<TensorNode>* vec = std::get_if<std::vector<TensorNode>>(&current_node->data);
        if (!vec || indices[i] >= (int)vec->size()) throw std::out_of_range("Index out of range.");
        current_node = &(*vec)[indices[i]];
    }
    return current_node;
}

void Matrix::flatten(const TensorNode& node, std::vector<float>& out) const {
    if (const float* val = std::get_if<float>(&node.data)) {
        out.push_back(*val);
    } else if (const std::vector<TensorNode>* vec = std::get_if<std::vector<TensorNode>>(&node.data)) {
        for (const auto& child_node : *vec) flatten(child_node, out);
    }
}

// --- Constructors ---

Matrix::Matrix(const std::vector<int>& shape) : shape_(shape) {
    if (shape.empty()) return;
    root_.resize(shape[0]);
    for(int i = 0; i < shape[0]; ++i) build_tensor(root_[i], shape, 1, false, false, 0.0f);
}

Matrix::Matrix(const std::vector<int>& shape, bool init_with_val, float val) : shape_(shape) {
    if (shape.empty()) return;
    root_.resize(shape[0]);
    for(int i = 0; i < shape[0]; ++i) build_tensor(root_[i], shape, 1, false, init_with_val, val);
}


// --- Public Methods ---

float& Matrix::at(const std::vector<int>& indices) { return std::get<float>(get_node(indices)->data); }
const float& Matrix::at(const std::vector<int>& indices) const { return std::get<float>(get_node(indices)->data); }

void Matrix::reshape(const std::vector<int>& new_shape) {
    std::vector<float> flat_data;
    for(const auto& node : root_) flatten(node, flat_data);

    long long new_size = 1;
    for (int dim : new_shape) new_size *= dim;
    if (new_size != (long long)flat_data.size()) throw std::invalid_argument("Incompatible shape size.");

    shape_ = new_shape;
    root_.clear();
    root_.resize(shape_[0]);
    for(int i = 0; i < shape_[0]; ++i) build_tensor(root_[i], shape_, 1, false, false, 0.0f);

    int flat_idx = 0;
    std::function<void(TensorNode&)> repopulate = [&](TensorNode& node) {
        if (std::holds_alternative<float>(node.data)) {
            node.data = flat_data[flat_idx++];
        } else {
            for (auto& child : std::get<std::vector<TensorNode>>(node.data)) repopulate(child);
        }
    };
    for(auto& node : root_) repopulate(node);
}

// --- Operators and Math ---

void Matrix::add_recursive(const TensorNode& a, const TensorNode& b, TensorNode& result) const {
    if (std::holds_alternative<float>(a.data)) {
        result.data = std::get<float>(a.data) + std::get<float>(b.data);
    } else {
        const auto& a_vec = std::get<std::vector<TensorNode>>(a.data);
        const auto& b_vec = std::get<std::vector<TensorNode>>(b.data);
        auto& res_vec = std::get<std::vector<TensorNode>>(result.data);
        for (size_t i = 0; i < a_vec.size(); ++i) add_recursive(a_vec[i], b_vec[i], res_vec[i]);
    }
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (this->shape_ != other.shape_) throw std::invalid_argument("Shape mismatch for addition.");
    Matrix result(this->shape_);
    for(size_t i = 0; i < root_.size(); ++i) add_recursive(root_[i], other.root_[i], result.root_[i]);
    return result;
}

void Matrix::add_assign_recursive(const TensorNode& other, TensorNode& self) {
    if (std::holds_alternative<float>(self.data)) {
        std::get<float>(self.data) += std::get<float>(other.data);
    } else {
        auto& self_vec = std::get<std::vector<TensorNode>>(self.data);
        const auto& other_vec = std::get<std::vector<TensorNode>>(other.data);
        for (size_t i = 0; i < self_vec.size(); ++i) add_assign_recursive(other_vec[i], self_vec[i]);
    }
}

Matrix& Matrix::operator+=(const Matrix& other) {
    if (this->shape_ != other.shape_) throw std::invalid_argument("Shape mismatch for addition assignment.");
    for(size_t i = 0; i < root_.size(); ++i) add_assign_recursive(other.root_[i], this->root_[i]);
    return *this;
}

void Matrix::add_scalar_recursive(const TensorNode& a, float scalar, TensorNode& result) const {
    if (std::holds_alternative<float>(a.data)) {
        result.data = std::get<float>(a.data) + scalar;
    } else {
        const auto& a_vec = std::get<std::vector<TensorNode>>(a.data);
        auto& res_vec = std::get<std::vector<TensorNode>>(result.data);
        for (size_t i = 0; i < a_vec.size(); ++i) add_scalar_recursive(a_vec[i], scalar, res_vec[i]);
    }
}

Matrix Matrix::operator+(float scalar) const {
    Matrix result(this->shape_);
    for(size_t i = 0; i < root_.size(); ++i) add_scalar_recursive(root_[i], scalar, result.root_[i]);
    return result;
}

void Matrix::mul_recursive(const TensorNode& a, const TensorNode& b, TensorNode& result) const {
    if (std::holds_alternative<float>(a.data)) {
        result.data = std::get<float>(a.data) * std::get<float>(b.data);
    } else {
        const auto& a_vec = std::get<std::vector<TensorNode>>(a.data);
        const auto& b_vec = std::get<std::vector<TensorNode>>(b.data);
        auto& res_vec = std::get<std::vector<TensorNode>>(result.data);
        for (size_t i = 0; i < a_vec.size(); ++i) mul_recursive(a_vec[i], b_vec[i], res_vec[i]);
    }
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (this->shape_ != other.shape_) throw std::invalid_argument("Shape mismatch for element-wise multiplication.");
    Matrix result(this->shape_);
    for(size_t i = 0; i < root_.size(); ++i) mul_recursive(root_[i], other.root_[i], result.root_[i]);
    return result;
}


void Matrix::div_recursive(const TensorNode& a, float scalar, TensorNode& result) const {
    if (std::holds_alternative<float>(a.data)) {
        result.data = std::get<float>(a.data) / scalar;
    } else {
        const auto& a_vec = std::get<std::vector<TensorNode>>(a.data);
        auto& res_vec = std::get<std::vector<TensorNode>>(result.data);
        for (size_t i = 0; i < a_vec.size(); ++i) div_recursive(a_vec[i], scalar, res_vec[i]);
    }
}

Matrix Matrix::operator/(float scalar) const {
    Matrix result(this->shape_);
    for(size_t i = 0; i < root_.size(); ++i) div_recursive(root_[i], scalar, result.root_[i]);
    return result;
}

Matrix Matrix::transpose(int dim1, int dim2) const {
    if (dim1 >= (int)shape_.size() || dim2 >= (int)shape_.size()) throw std::out_of_range("Invalid dimensions for transpose.");

    std::vector<int> new_shape = shape_;
    std::swap(new_shape[dim1], new_shape[dim2]);
    Matrix result(new_shape);

    std::vector<int> old_indices(shape_.size());
    std::function<void(int)> recurse =
        [&](int k) {
        if (k == (int)shape_.size()) {
            std::vector<int> new_indices = old_indices;
            std::swap(new_indices[dim1], new_indices[dim2]);
            result.at(new_indices) = this->at(old_indices);
            return;
        }
        for (int i = 0; i < shape_[k]; ++i) {
            old_indices[k] = i;
            recurse(k + 1);
        }
    };
    recurse(0);
    return result;
}

Matrix Matrix::sum(int axis) const {
    if (axis < 0 || axis >= (int)shape_.size()) throw std::invalid_argument("Invalid axis for sum.");
    std::vector<int> new_shape = shape_;
    new_shape.erase(new_shape.begin() + axis);
    if (new_shape.empty()) new_shape.push_back(1);
    Matrix result(new_shape);
    // Simplified sum: flatten and sum all
    std::vector<float> flat;
    flatten(root_[0], flat); // Assuming first dimension is 1
    float total = std::accumulate(flat.begin(), flat.end(), 0.0f);
    result.at({0}) = total;
    return result;
}

Matrix Matrix::mean(int axis) const {
     return this->sum(axis) / shape_[axis];
}

Matrix Matrix::variance(int axis) const {
    Matrix mean_val = this->mean(axis);
    // This is a simplified implementation. Broadcasting is needed for a correct one.
    Matrix diff = *this + (mean_val.at({0}) * -1);
    Matrix sq_diff = diff * diff;
    return sq_diff.mean(axis);
}

// --- Static Methods ---

Matrix Matrix::random(const std::vector<int>& shape){
    Matrix m(shape);
    m.root_.clear();
    m.root_.resize(shape[0]);
    for(int i = 0; i < shape[0]; ++i) m.build_tensor(m.root_[i], shape, 1, true, false, 0.0f);
    return m;
}

Matrix Matrix::ones(const std::vector<int>& shape){
    return Matrix(shape, true, 1.0f);
}

Matrix Matrix::matmul(const Matrix& a, const Matrix& b) {
    if (a.shape().size() != 2 || b.shape().size() != 2) throw std::invalid_argument("Matmul only supports 2D matrices.");
    if (a.shape()[1] != b.shape()[0]) throw std::invalid_argument("Matmul dimensions are incompatible.");

    Matrix result({a.shape()[0], b.shape()[1]});
    for (int i = 0; i < a.shape()[0]; ++i) {
        for (int j = 0; j < b.shape()[1]; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < a.shape()[1]; ++k) sum += a.at({i, k}) * b.at({k, j});
            result.at({i, j}) = sum;
        }
    }
    return result;
}

Matrix Matrix::softmax(const Matrix& m, int axis) {
    if (m.shape().size() != 2 || (axis != -1 && axis != 1)) throw std::invalid_argument("Softmax only supports 2D matrices on the last axis.");

    Matrix result(m.shape());
    for (int i = 0; i < m.shape()[0]; ++i) {
        float max_val = m.at({i, 0});
        for (int j = 1; j < m.shape()[1]; ++j) if (m.at({i, j}) > max_val) max_val = m.at({i, j});

        float sum = 0.0f;
        for (int j = 0; j < m.shape()[1]; ++j) {
            float exp_val = std::exp(m.at({i, j}) - max_val);
            result.at({i, j}) = exp_val;
            sum += exp_val;
        }
        for (int j = 0; j < m.shape()[1]; ++j) result.at({i, j}) /= sum;
    }
    return result;
}

void Matrix::sqrt_recursive(const TensorNode& a, TensorNode& result) const {
    if (std::holds_alternative<float>(a.data)) {
        result.data = std::sqrt(std::get<float>(a.data));
    } else {
        const auto& a_vec = std::get<std::vector<TensorNode>>(a.data);
        auto& res_vec = std::get<std::vector<TensorNode>>(result.data);
        for (size_t i = 0; i < a_vec.size(); ++i) sqrt_recursive(a_vec[i], res_vec[i]);
    }
}

Matrix Matrix::sqrt(const Matrix& m){
    Matrix result(m.shape());
    for(size_t i=0; i < m.root_.size(); ++i) result.sqrt_recursive(m.root_[i], result.root_[i]);
    return result;
}

} // namespace TissNum
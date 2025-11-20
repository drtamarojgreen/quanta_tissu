#include "matrix.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <functional>
#include <vector>
#include <thread>

namespace TissNum {

// The old constructor is removed as the default constructor now handles initialization.

float& Matrix::operator()(const std::vector<size_t>& indices) {
    if (indices.size() != 2) {
        throw std::out_of_range("Index dimension mismatch for sparse matrix (must be 2).");
    }
    size_t r = indices[0];
    size_t c = indices[1];
    if (r >= 4 || c >= 4) {
        throw std::out_of_range("Index out of range for 4x4 sparse matrix.");
    }
    if (!data_[r][c].has_value()) {
        // This is a design decision. Accessing a non-existent element creates it.
        // A different design might throw an error.
        data_[r][c] = 0.0f;
    }
    return *data_[r][c];
}

const float& Matrix::operator()(const std::vector<size_t>& indices) const {
    if (indices.size() != 2) {
        throw std::out_of_range("Index dimension mismatch for sparse matrix (must be 2).");
    }
    size_t r = indices[0];
    size_t c = indices[1];
    if (r >= 4 || c >= 4) {
        throw std::out_of_range("Index out of range for 4x4 sparse matrix.");
    }
    if (!data_[r][c].has_value()) {
        // This is a design decision. Accessing a non-existent element on a const object is an error.
        throw std::runtime_error("Attempted to access non-existent element in const sparse matrix.");
    }
    return *data_[r][c];
}

Matrix Matrix::random(const std::vector<size_t>& shape) {
    // Shape is ignored in the new design. Returns a fully populated 4x4 random matrix.
    Matrix m;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            m.data_[r][c] = dist(gen);
        }
    }
    return m;
}

Matrix Matrix::zeros(const std::vector<size_t>& shape) {
    // Shape is ignored. Returns a default-constructed (empty) matrix.
    return Matrix();
}

Matrix Matrix::ones(const std::vector<size_t>& shape) {
    // Shape is ignored. Returns a fully populated 4x4 matrix of ones.
    Matrix m;
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            m.data_[r][c] = 1.0f;
        }
    }
    return m;
}

Matrix Matrix::transpose() const {
    // For the sparse 4x4 matrix, transpose is always a 2D operation.
    return transpose(0, 1);
}

Matrix Matrix::reshape(const std::vector<size_t>& new_shape) const {
    // Reshaping a sparse, fixed-grid matrix is an ambiguous operation.
    // This implementation interprets it as creating a new matrix and populating
    // it from a flattened list of the source's non-empty values.
    Matrix result;
    std::vector<float> values;
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            if (data_[r][c].has_value()) {
                values.push_back(*data_[r][c]);
            }
        }
    }

    size_t value_idx = 0;
    for (size_t r = 0; r < new_shape[0] && r < 4; ++r) {
        for (size_t c = 0; c < new_shape[1] && c < 4; ++c) {
            if (value_idx < values.size()) {
                result.data_[r][c] = values[value_idx++];
            }
        }
    }

    return result;
}

Matrix Matrix::transpose(int dim1, int dim2) const {
    if ((dim1 == 0 && dim2 == 1) || (dim1 == 1 && dim2 == 0)) {
        Matrix result;
        for (size_t r = 0; r < 4; ++r) {
            for (size_t c = 0; c < 4; ++c) {
                result.data_[r][c] = data_[c][r];
            }
        }
        return result;
    }
    // Transposing other dimensions is not meaningful for a fixed 2D grid.
    throw std::invalid_argument("Transpose for sparse matrix only supports swapping dimensions 0 and 1.");
}

Matrix Matrix::operator+(const Matrix& other) const {
    return broadcast_op(other, std::plus<float>());
}

Matrix Matrix::operator-(const Matrix& other) const {
    return broadcast_op(other, std::minus<float>());
}

Matrix Matrix::operator*(const Matrix& other) const {
    return broadcast_op(other, std::multiplies<float>());
}

Matrix Matrix::operator/(const Matrix& other) const {
    return broadcast_op(other, std::divides<float>());
}

Matrix Matrix::matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() == 2 && b.get_shape().size() == 2) {
        if (a.cols() != b.rows()) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 2D multiplication.");
        }
        Matrix result({a.rows(), b.cols()});

        unsigned int num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        size_t rows_per_thread = a.rows() / num_threads;

        for (unsigned int i = 0; i < num_threads; ++i) {
            size_t start_row = i * rows_per_thread;
            size_t end_row = (i == num_threads - 1) ? a.rows() : start_row + rows_per_thread;

            threads.emplace_back([&, start_row, end_row]() {
                for (size_t r = start_row; r < end_row; ++r) {
                    for (size_t c = 0; c < b.cols(); ++c) {
                        for (size_t k = 0; k < a.cols(); ++k) {
                            result({r, c}) += a({r, k}) * b({k, c});
                        }
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
        return result;
    }

    if (a.get_shape().size() == 3 && b.get_shape().size() == 2) {
        if (a.get_shape()[2] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 3D x 2D multiplication.");
        }
        size_t batch_size = a.get_shape()[0];
        size_t seq_len = a.get_shape()[1];
        size_t dim = a.get_shape()[2];
        size_t new_dim = b.get_shape()[1];

        Matrix result({batch_size, seq_len, new_dim});
        for (size_t i = 0; i < batch_size; ++i) {
            for (size_t j = 0; j < seq_len; ++j) {
                for (size_t k = 0; k < new_dim; ++k) {
                    float sum = 0.0f;
                    for (size_t l = 0; l < dim; ++l) {
                        sum += a({i, j, l}) * b({l, k});
                    }
                    result({i, j, k}) = sum;
                }
            }
        }
        return result;
    }

    if (a.get_shape().size() == 1 && b.get_shape().size() == 2) {
        if (a.get_shape()[0] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 1D x 2D multiplication.");
        }
        size_t dim = a.get_shape()[0];
        size_t new_dim = b.get_shape()[1];

        Matrix result({new_dim});
        for (size_t i = 0; i < new_dim; ++i) {
            float sum = 0.0f;
            for (size_t j = 0; j < dim; ++j) {
                sum += a({j}) * b({j, i});
            }
            result({i}) = sum;
        }
        return result;
    }

    if (a.get_shape().size() == 2 && b.get_shape().size() == 1) {
        if (a.get_shape()[1] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 2D x 1D multiplication.");
        }
        size_t rows = a.get_shape()[0];
        size_t cols = a.get_shape()[1];

        Matrix result({rows});
        for (size_t i = 0; i < rows; ++i) {
            float sum = 0.0f;
            for (size_t j = 0; j < cols; ++j) {
                sum += a({i, j}) * b({j});
            }
            result({i}) = sum;
        }
        return result;
    }

    throw std::invalid_argument("matmul not implemented for these matrix shapes.");
}

Matrix Matrix::sum(int axis) const {
    if (axis == -1) {
        Matrix result({1});
        result.data_[0] = std::accumulate(data_.begin(), data_.end(), 0.0f);
        return result;
    }

    std::vector<size_t> new_shape = shape_;
    new_shape[axis] = 1;
    Matrix result(new_shape);
    result.data_.assign(result.data_.size(), 0.0f);

    std::vector<size_t> indices(shape_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
        size_t temp_i = i;
        for (int d = shape_.size() - 1; d >= 0; --d) {
            indices[d] = temp_i % shape_[d];
            temp_i /= shape_[d];
        }

        std::vector<size_t> new_indices = indices;
        new_indices[axis] = 0;
        result(new_indices) += data_[i];
    }
    return result;
}

Matrix Matrix::mean(int axis) const {
    Matrix s = sum(axis);
    if (axis == -1) {
        return s / data_.size();
    }
    return s / shape_[axis];
}

Matrix Matrix::variance(int axis) const {
    if (data_.empty()) {
        throw std::invalid_argument("Cannot compute variance of an empty matrix.");
    }

    if (axis == -1) {
        // Welford's algorithm for numerical stability
        float mean_val = 0.0f;
        float M2 = 0.0f;
        size_t count = 0;

        for (size_t i = 0; i < data_.size(); ++i) {
            count++;
            float delta = data_[i] - mean_val;
            mean_val += delta / count;
            M2 += delta * (data_[i] - mean_val);
        }
        Matrix result({1});
        result.data_[0] = M2 / count;
        return result;
    }

    // For axis-wise variance, we'll use the two-pass approach for now, but ensure correct division.
    Matrix m = mean(axis);
    Matrix diff = (*this) - m;
    Matrix sq_diff = diff * diff;
    Matrix sum_sq_diff = sq_diff.sum(axis);

    std::vector<size_t> new_shape = shape_;
    new_shape[axis] = 1;
    Matrix result(new_shape);

    size_t elements_in_axis = shape_[axis];
    if (elements_in_axis == 0) {
        throw std::invalid_argument("Cannot compute variance along an axis with zero elements.");
    }

    for (size_t i = 0; i < result.data_.size(); ++i) {
        result.data_[i] = sum_sq_diff.data_[i] / elements_in_axis;
    }
    return result;
}

Matrix Matrix::max(int axis) const {
    if (axis == -1) {
        Matrix result({1});
        result.data_[0] = *std::max_element(data_.begin(), data_.end());
        return result;
    }

    std::vector<size_t> new_shape = shape_;
    new_shape[axis] = 1;
    Matrix result(new_shape);
    result.data_.assign(result.data_.size(), -std::numeric_limits<float>::infinity());

    std::vector<size_t> indices(shape_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
        size_t temp_i = i;
        for (int d = shape_.size() - 1; d >= 0; --d) {
            indices[d] = temp_i % shape_[d];
            temp_i /= shape_[d];
        }

        std::vector<size_t> new_indices = indices;
        new_indices[axis] = 0;
        if (data_[i] > result(new_indices)) {
            result(new_indices) = data_[i];
        }
    }
    return result;
}

Matrix Matrix::sqrt(const Matrix& m) {
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::sqrt(m.data_[i]);
    }
    return result;
}

Matrix Matrix::pow(const Matrix& m, float exponent) {
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::pow(m.data_[i], exponent);
    }
    return result;
}

Matrix Matrix::exp(const Matrix& m) {
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = std::exp(m.data_[i]);
    }
    return result;
}

Matrix Matrix::operator+(float scalar) const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + scalar;
    }
    return result;
}

Matrix Matrix::operator-(float scalar) const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - scalar;
    }
    return result;
}

Matrix Matrix::operator*(float scalar) const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * scalar;
    }
    return result;
}

Matrix Matrix::operator/(float scalar) const {
    if (scalar == 0.0f) {
        throw std::invalid_argument("Division by zero");
    }
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] / scalar;
    }
    return result;
}

Matrix operator/(float scalar, const Matrix& m) {
    if (scalar == 0.0f) {
        throw std::invalid_argument("Division by zero");
    }
    Matrix result(m.shape_);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        result.data_[i] = scalar / m.data_[i];
    }
    return result;
}

Matrix Matrix::element_wise_product(const Matrix& other) const {
    return (*this) * other;
}

Matrix Matrix::element_wise_division(const Matrix& other) const {
    return (*this) / other;
}

Matrix Matrix::element_wise_sqrt() const {
    Matrix result(shape_);
    for (size_t i = 0; i < data_.size(); ++i) {
        if (data_[i] < 0.0f) {
            throw std::invalid_argument("Cannot take square root of negative number in element_wise_sqrt.");
        }
        result.data_[i] = std::sqrt(data_[i]);
    }
    return result;
}

Matrix Matrix::concatenate(const Matrix& a, const Matrix& b, int axis) {
    if (a.shape_.size() != b.shape_.size()) {
        throw std::invalid_argument("Matrices must have the same number of dimensions to concatenate.");
    }
    for (size_t i = 0; i < a.shape_.size(); ++i) {
        if (i != axis && a.shape_[i] != b.shape_[i]) {
            throw std::invalid_argument("Matrix dimensions must match for concatenation, except for the concatenation axis.");
        }
    }

    std::vector<size_t> new_shape = a.shape_;
    new_shape[axis] += b.shape_[axis];
    Matrix result(new_shape);

    size_t outer_dims = 1;
    for (int i = 0; i < axis; ++i) {
        outer_dims *= a.shape_[i];
    }

    size_t a_axis_size = a.shape_[axis];
    size_t b_axis_size = b.shape_[axis];

    size_t inner_dims = 1;
    for (size_t i = axis + 1; i < a.shape_.size(); ++i) {
        inner_dims *= a.shape_[i];
    }

    for (size_t i = 0; i < outer_dims; ++i) {
        float* dest_ptr = result.get_data() + i * (a_axis_size + b_axis_size) * inner_dims;
        const float* a_src_ptr = a.get_data() + i * a_axis_size * inner_dims;
        std::copy(a_src_ptr, a_src_ptr + a_axis_size * inner_dims, dest_ptr);

        dest_ptr += a_axis_size * inner_dims;
        const float* b_src_ptr = b.get_data() + i * b_axis_size * inner_dims;
        std::copy(b_src_ptr, b_src_ptr + b_axis_size * inner_dims, dest_ptr);
    }

    return result;
}

Matrix operator*(float scalar, const Matrix& m) {
    return m * scalar;
}

Matrix Matrix::batch_matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() != 4 || b.get_shape().size() != 4) {
        throw std::invalid_argument("batch_matmul only supports 4D matrices.");
    }
    if (a.get_shape()[0] != b.get_shape()[0] || a.get_shape()[1] != b.get_shape()[1]) {
        throw std::invalid_argument("Batch dimensions must match for batch_matmul.");
    }
    if (a.get_shape()[3] != b.get_shape()[2]) {
        throw std::invalid_argument("Matrix dimensions are not compatible for multiplication.");
    }

    size_t batch_size = a.get_shape()[0];
    size_t num_heads = a.get_shape()[1];
    size_t seq_len_q = a.get_shape()[2];
    size_t seq_len_k = b.get_shape()[3];
    size_t head_dim = a.get_shape()[3];

    Matrix result({batch_size, num_heads, seq_len_q, seq_len_k});

    for (size_t i = 0; i < batch_size; ++i) {
        for (size_t j = 0; j < num_heads; ++j) {
            for (size_t l = 0; l < seq_len_q; ++l) {
                for (size_t m = 0; m < seq_len_k; ++m) {
                    float sum = 0.0f;
                    for (size_t n = 0; n < head_dim; ++n) {
                        sum += a({i, j, l, n}) * b({i, j, n, m});
                    }
                    result({i, j, l, m}) = sum;
                }
            }
        }
    }
    return result;
}

Matrix Matrix::matmul(const Matrix& other) const {
    if (shape_.size() == 2 && other.shape_.size() == 2) {
        if (shape_[1] != other.shape_[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 2D multiplication.");
        }
        Matrix result({shape_[0], other.shape_[1]});
        for (size_t i = 0; i < shape_[0]; ++i) {
            for (size_t j = 0; j < other.shape_[1]; ++j) {
                float sum = 0;
                for (size_t k = 0; k < shape_[1]; ++k) {
                    sum += (*this)({i, k}) * other({k, j});
                }
                result({i, j}) = sum;
            }
        }
        return result;
    } else if (shape_.size() == 3 && other.shape_.size() == 2) {
        if (shape_[2] != other.shape_[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 3D x 2D multiplication.");
        }
        Matrix result({shape_[0], shape_[1], other.shape_[1]});
        for (size_t b = 0; b < shape_[0]; ++b) {
            for (size_t i = 0; i < shape_[1]; ++i) {
                for (size_t j = 0; j < other.shape_[1]; ++j) {
                    float sum = 0;
                    for (size_t k = 0; k < shape_[2]; ++k) {
                        sum += (*this)({b, i, k}) * other({k, j});
                    }
                    result({b, i, j}) = sum;
                }
            }
        }
        return result;
    } else if (shape_.size() == 3 && other.shape_.size() == 3) {
        if (shape_[0] != other.shape_[0]) {
            throw std::invalid_argument("Batch sizes must be equal for batch matrix multiplication.");
        }
        if (shape_[2] != other.shape_[1]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for batch multiplication.");
        }
        Matrix result({shape_[0], shape_[1], other.shape_[2]});
        for (size_t b = 0; b < shape_[0]; ++b) {
            for (size_t i = 0; i < shape_[1]; ++i) {
                for (size_t j = 0; j < other.shape_[2]; ++j) {
                    float sum = 0;
                    for (size_t k = 0; k < shape_[2]; ++k) {
                        sum += (*this)({b, i, k}) * other({b, k, j});
                    }
                    result({b, i, j}) = sum;
                }
            }
        }
        return result;
    } else {
        throw std::invalid_argument("Unsupported matrix multiplication dimensions.");
    }
}

Matrix Matrix::batch_matmul(const Matrix& other) const {
    if (shape_.size() != 3 || other.shape_.size() != 3) {
        throw std::invalid_argument("Batch matrix multiplication (batch_matmul) is only supported for 3D matrices.");
    }
    if (shape_[0] != other.shape_[0]) {
        throw std::invalid_argument("Batch sizes must be equal for batch matrix multiplication.");
    }
    if (shape_[2] != other.shape_[1]) {
        throw std::invalid_argument("Matrix dimensions are not compatible for batch multiplication.");
    }

    Matrix result({shape_[0], shape_[1], other.shape_[2]});
    for (size_t b = 0; b < shape_[0]; ++b) {
        for (size_t i = 0; i < shape_[1]; ++i) {
            for (size_t j = 0; j < other.shape_[2]; ++j) {
                float sum = 0;
                for (size_t k = 0; k < shape_[2]; ++k) {
                    sum += (*this)({b, i, k}) * other({b, k, j});
                }
                result({b, i, j}) = sum;
            }
        }
    }
    return result;
}

} // namespace TissNum

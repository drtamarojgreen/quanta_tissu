#include "matrix.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <functional>
#include <vector>
#include <thread>

namespace TissNum {

Matrix::Matrix(const std::vector<size_t>& shape) : shape_(shape) {
    size_t total_size = 1;
    for (size_t dim : shape) {
        total_size *= dim;
    }
    data_.resize(total_size, 0.0f);
}

float& Matrix::operator()(const std::vector<size_t>& indices) {
    if (indices.size() != shape_.size()) {
        throw std::out_of_range("Index dimension mismatch.");
    }
    size_t index = 0;
    for (size_t i = 0; i < shape_.size(); ++i) {
        if (indices[i] >= shape_[i]) {
            throw std::out_of_range("Index out of range.");
        }
        size_t stride = 1;
        for (size_t j = i + 1; j < shape_.size(); ++j) {
            stride *= shape_[j];
        }
        index += indices[i] * stride;
    }
    return data_[index];
}

const float& Matrix::operator()(const std::vector<size_t>& indices) const {
    if (indices.size() != shape_.size()) {
        throw std::out_of_range("Index dimension mismatch.");
    }
    size_t index = 0;
    for (size_t i = 0; i < shape_.size(); ++i) {
        if (indices[i] >= shape_[i]) {
            throw std::out_of_range("Index out of range.");
        }
        size_t stride = 1;
        for (size_t j = i + 1; j < shape_.size(); ++j) {
            stride *= shape_[j];
        }
        index += indices[i] * stride;
    }
    return data_[index];
}

Matrix Matrix::random(const std::vector<size_t>& shape) {
    Matrix m(shape);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    for (size_t i = 0; i < m.data_.size(); ++i) {
        m.data_[i] = dist(gen);
    }
    return m;
}

Matrix Matrix::zeros(const std::vector<size_t>& shape) {
    return Matrix(shape);
}

Matrix Matrix::ones(const std::vector<size_t>& shape) {
    Matrix m(shape);
    std::fill(m.data_.begin(), m.data_.end(), 1.0f);
    return m;
}

Matrix Matrix::transpose() const {
    if (shape_.size() != 2) {
        throw std::invalid_argument("Default transpose is only supported for 2D matrices.");
    }
    return transpose(0, 1);
}

Matrix Matrix::reshape(const std::vector<size_t>& new_shape) const {
    size_t total_size = 1;
    for (size_t dim : new_shape) {
        total_size *= dim;
    }

    if (total_size != data_.size()) {
        assert_size_match(data_.size(), total_size);
    }
    Matrix result(new_shape);
    result.data_ = data_;
    return result;
}

Matrix Matrix::transpose(int dim1, int dim2) const {
    // Validate the input dimensions. dim1 and dim2 must be valid indices within the matrix's shape.
    // If they are out of bounds, an std::out_of_range exception is thrown.
    if (dim1 >= shape_.size() || dim2 >= shape_.size()) {
        throw std::out_of_range("Invalid dimensions for transpose.");
    }

    // Create a new shape vector for the transposed matrix.
    // This new shape is initially a copy of the original shape.
    std::vector<size_t> new_shape = shape_;
    // Swap the dimensions at dim1 and dim2 in the new shape.
    // For example, if original shape is {A, B, C} and dim1=0, dim2=1, new_shape becomes {B, A, C}.
    std::swap(new_shape[dim1], new_shape[dim2]);

    // Create a new Matrix object with the new, transposed shape.
    // This 'result' matrix will store the transposed data.
    Matrix result(new_shape);

    // 'old_indices' is a temporary vector used to store the current multi-dimensional index
    // for the original matrix. Its size matches the number of dimensions of the original matrix.
    std::vector<size_t> old_indices(shape_.size());

    // 'recurse' is a recursive lambda function that iterates through all elements of the original matrix.
    // It maps each element from its original position to its new position in the transposed matrix.
    // 'k' represents the current dimension being processed in the recursion.
    // Iterative implementation to avoid stack overflow for high-dimensional tensors.
    // This approach calculates the new data position for each element iteratively.

    // Pre-calculate strides for both old and new shapes for efficient index calculation.
    std::vector<size_t> old_strides(shape_.size());
    if (!shape_.empty()) {
        old_strides.back() = 1;
        for (int i = shape_.size() - 2; i >= 0; --i) {
            old_strides[i] = old_strides[i + 1] * shape_[i + 1];
        }
    }

    std::vector<size_t> new_strides(new_shape.size());
    if (!new_shape.empty()) {
        new_strides.back() = 1;
        for (int i = new_shape.size() - 2; i >= 0; --i) {
            new_strides[i] = new_strides[i + 1] * new_shape[i + 1];
        }
    }

    // Iterate through each element in the 1D data vector.
    for (size_t i = 0; i < data_.size(); ++i) {
        // Convert the 1D index 'i' back to multi-dimensional 'old_indices'.
        std::vector<size_t> old_indices(shape_.size());
        size_t remaining_i = i;
        for (size_t j = 0; j < shape_.size(); ++j) {
            old_indices[j] = remaining_i / old_strides[j];
            remaining_i %= old_strides[j];
        }

        // Calculate the 'new_indices' by swapping the dimensions.
        std::vector<size_t> new_indices = old_indices;
        std::swap(new_indices[dim1], new_indices[dim2]);

        // Convert the 'new_indices' back to a 1D 'new_index'.
        size_t new_index = 0;
        for (size_t j = 0; j < new_shape.size(); ++j) {
            new_index += new_indices[j] * new_strides[j];
        }
        // Assign the data to the new position.
        result.data_[new_index] = data_[i];
    }

    // Return the newly created and populated transposed matrix.
    return result;
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

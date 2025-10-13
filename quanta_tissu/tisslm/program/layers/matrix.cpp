#include "matrix.h"
#include <stdexcept>
#include <random>

Matrix::Matrix(int rows, int cols) : rows(rows), cols(cols), data(rows * cols, 0.0f) {}

Matrix::Matrix(const std::vector<std::vector<float>>& initial_data) {
    if (initial_data.empty()) {
        rows = 0;
        cols = 0;
        return;
    }
    rows = initial_data.size();
    cols = initial_data[0].size();
    data.reserve(rows * cols);
    for (const auto& row_vec : initial_data) {
        data.insert(data.end(), row_vec.begin(), row_vec.end());
    }
}

int Matrix::get_rows() const { return rows; }
int Matrix::get_cols() const { return cols; }

float& Matrix::at(int row, int col) {
    if (row >= rows || col >= cols) {
        throw std::out_of_range("Matrix access out of range");
    }
    return data[row * cols + col];
}

const float& Matrix::at(int row, int col) const {
    if (row >= rows || col >= cols) {
        throw std::out_of_range("Matrix access out of range");
    }
    return data[row * cols + col];
}

Matrix Matrix::random(int rows, int cols) {
    Matrix m(rows, cols);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 1);
    for (int i = 0; i < rows * cols; ++i) {
        m.data[i] = d(gen);
    }
    return m;
}

Matrix Matrix::zeros(int rows, int cols) {
    return Matrix(rows, cols);
}

Matrix Matrix::transpose() const {
    Matrix result(cols, rows);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result.at(j, i) = at(i, j);
        }
    }
    return result;
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) {
        throw std::invalid_argument("Matrix dimensions must match for addition.");
    }
    Matrix result(rows, cols);
    for (int i = 0; i < rows * cols; ++i) {
        result.data[i] = data[i] + other.data[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) {
        throw std::invalid_argument("Matrix dimensions must match for subtraction.");
    }
    Matrix result(rows, cols);
    for (int i = 0; i < rows * cols; ++i) {
        result.data[i] = data[i] - other.data[i];
    }
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) {
        throw std::invalid_argument("Matrix dimensions must match for element-wise multiplication.");
    }
    Matrix result(rows, cols);
    for (int i = 0; i < rows * cols; ++i) {
        result.data[i] = data[i] * other.data[i];
    }
    return result;
}

Matrix Matrix::dot(const Matrix& other) const {
    if (cols != other.rows) {
        throw std::invalid_argument("Matrix dimensions are not compatible for dot product.");
    }
    Matrix result(rows, other.cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < other.cols; ++j) {
            for (int k = 0; k < cols; ++k) {
                result.at(i, j) += at(i, k) * other.at(k, j);
            }
        }
    }
    return result;
}

void Matrix::print() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << at(i, j) << " ";
        }
        std::cout << std::endl;
    }
}
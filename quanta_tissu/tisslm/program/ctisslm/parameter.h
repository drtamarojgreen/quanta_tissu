#ifndef QUANTA_TISSU_PARAMETER_H
#define QUANTA_TISSU_PARAMETER_H

#include <vector>
#include <string>
#include <numeric>
#include <random>
#include <stdexcept>

namespace quanta_tissu {

/**
 * @brief Parameter class for model weights and gradients.
 * Supports 1D (vector) and 2D (matrix) operations with bounds checking.
 */
class Parameter {
public:
    std::vector<double> value;
    std::vector<double> grad;
    std::vector<size_t> shape;
    std::string name;

    Parameter(size_t size, const std::string& param_name = "") 
        : value(size), grad(size), shape({size}), name(param_name) {
        std::fill(value.begin(), value.end(), 0.0);
        std::fill(grad.begin(), grad.end(), 0.0);
    }

    Parameter(size_t rows, size_t cols, const std::string& param_name = "")
        : value(rows * cols), grad(rows * cols), shape({rows, cols}), name(param_name) {
        std::fill(value.begin(), value.end(), 0.0);
        std::fill(grad.begin(), grad.end(), 0.0);
    }

    Parameter(const std::vector<size_t>& param_shape, const std::string& param_name = "")
        : shape(param_shape), name(param_name) {
        size_t total_size = 1;
        for (size_t dim : param_shape) total_size *= dim;
        value.resize(total_size);
        grad.resize(total_size);
        std::fill(value.begin(), value.end(), 0.0);
        std::fill(grad.begin(), grad.end(), 0.0);
    }

    double& operator[](size_t index) {
        if (shape.size() != 1 || index >= value.size()) {
            throw std::out_of_range("Parameter: 1D index out of bounds.");
        }
        return value[index];
    }

    double& operator()(size_t row, size_t col) {
        if (shape.size() != 2 || row >= shape[0] || col >= shape[1]) {
            throw std::out_of_range("Parameter: 2D index out of bounds.");
        }
        return value[row * shape[1] + col];
    }

    const double& operator[](size_t index) const {
        if (shape.size() != 1 || index >= value.size()) {
            throw std::out_of_range("Parameter: 1D index out of bounds.");
        }
        return value[index];
    }

    const double& operator()(size_t row, size_t col) const {
        if (shape.size() != 2 || row >= shape[0] || col >= shape[1]) {
            throw std::out_of_range("Parameter: 2D index out of bounds.");
        }
        return value[row * shape[1] + col];
    }

    size_t size() const { return value.size(); }

    void reshape(const std::vector<size_t>& new_shape) {
        size_t new_total_size = 1;
        for (size_t dim : new_shape) new_total_size *= dim;
        if (new_total_size != value.size()) {
            throw std::invalid_argument("Parameter: Reshape size mismatch.");
        }
        shape = new_shape;
    }

    void fill_random_normal(double mean = 0.0, double stddev = 0.01, unsigned int seed = 42) {
        std::mt19937 gen(seed);
        std::normal_distribution<double> dist(mean, stddev);
        for (double& val : value) val = dist(gen);
    }

    void fill_zeros() { std::fill(value.begin(), value.end(), 0.0); }
    void zero_grad() { std::fill(grad.begin(), grad.end(), 0.0); }
};

}

#endif // QUANTA_TISSU_PARAMETER_H

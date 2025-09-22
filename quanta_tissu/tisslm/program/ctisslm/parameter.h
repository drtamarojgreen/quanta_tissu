#ifndef QUANTA_TISSU_PARAMETER_H
#define QUANTA_TISSU_PARAMETER_H

#include <vector>
#include <string>
#include <numeric> // For std::iota

// For now, we'll use std::vector for simplicity. 
// In a real implementation, you'd likely use a dedicated linear algebra library like Eigen.

namespace quanta_tissu {

class Parameter {
public:
    std::vector<double> value; // Stores the parameter values
    std::vector<double> grad;  // Stores the gradients
    std::vector<size_t> shape; // Stores the shape of the parameter
    std::string name;          // Name of the parameter

    // Constructor for a 1D parameter (vector)
    Parameter(size_t size, const std::string& param_name = "") 
        : value(size), grad(size), shape({size}), name(param_name) {
        // Initialize with random values (e.g., using a normal distribution)
        // For simplicity, initializing with zeros for now.
        std::fill(value.begin(), value.end(), 0.0);
        std::fill(grad.begin(), grad.end(), 0.0);
    }

    // Constructor for a 2D parameter (matrix)
    Parameter(size_t rows, size_t cols, const std::string& param_name = "")
        : value(rows * cols), grad(rows * cols), shape({rows, cols}), name(param_name) {
        // Initialize with random values
        std::fill(value.begin(), value.end(), 0.0);
        std::fill(grad.begin(), grad.end(), 0.0);
    }

    // Constructor for a general N-D parameter
    Parameter(const std::vector<size_t>& param_shape, const std::string& param_name = "")
        : shape(param_shape), name(param_name) {
        size_t total_size = 1;
        for (size_t dim : param_shape) {
            total_size *= dim;
        }
        value.resize(total_size);
        grad.resize(total_size);
        std::fill(value.begin(), value.end(), 0.0);
        std::fill(grad.begin(), grad.end(), 0.0);
    }

    // Get element by index (for 1D)
    double& operator[](size_t index) {
        if (shape.size() != 1 || index >= value.size()) {
            // Handle error or throw exception
            // For simplicity, returning a dummy value or asserting
        }
        return value[index];
    }

    // Get element by row, col (for 2D)
    double& operator()(size_t row, size_t col) {
        if (shape.size() != 2 || row >= shape[0] || col >= shape[1]) {
            // Handle error or throw exception
        }
        return value[row * shape[1] + col];
    }

    // Get const element by index (for 1D)
    const double& operator[](size_t index) const {
        if (shape.size() != 1 || index >= value.size()) {
            // Handle error or throw exception
        }
        return value[index];
    }

    // Get const element by row, col (for 2D)
    const double& operator()(size_t row, size_t col) const {
        if (shape.size() != 2 || row >= shape[0] || col >= shape[1]) {
            // Handle error or throw exception
        }
        return value[row * shape[1] + col];
    }

    // Method to get total size
    size_t size() const {
        return value.size();
    }

    // Method to reshape (simple version, doesn't change data)
    void reshape(const std::vector<size_t>& new_shape) {
        size_t new_total_size = 1;
        for (size_t dim : new_shape) {
            new_total_size *= dim;
        }
        if (new_total_size != value.size()) {
            // Error: new shape size must match current total size
            return;
        }
        shape = new_shape;
    }

    // Method to fill with random normal values (placeholder)
    void fill_random_normal() {
        // This would require a random number generator
        // For now, just filling with a dummy value
        std::fill(value.begin(), value.end(), 0.01);
    }

    // Method to fill with zeros
    void fill_zeros() {
        std::fill(value.begin(), value.end(), 0.0);
    }

    // Method to fill gradients with zeros
    void zero_grad() {
        std::fill(grad.begin(), grad.end(), 0.0);
    }
};

} // namespace quanta_tissu

#endif // QUANTA_TISSU_PARAMETER_H

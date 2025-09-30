#pragma once

#include "matrix.h"

namespace TissNum {

class Parameter {
public:
    // Constructor
    Parameter(const Matrix& value, const std::string& name = "") : value_(value), grad_(Matrix::zeros(value.rows(), value.cols())), name_(name) {}

    // Accessors
    Matrix& value() { return value_; }
    const Matrix& value() const { return value_; }
    Matrix& grad() { return grad_; }
    const Matrix& grad() const { return grad_; }
    const std::string& name() const { return name_; }

private:
    Matrix value_;
    Matrix grad_;
    std::string name_;
};

} // namespace TissNum

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>

class Matrix {
public:
    Matrix(int rows, int cols);
    Matrix(const std::vector<std::vector<float>>& data);

    int get_rows() const;
    int get_cols() const;

    float& at(int row, int col);
    const float& at(int row, int col) const;

    static Matrix random(int rows, int cols);
    static Matrix zeros(int rows, int cols);

    Matrix transpose() const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const; // Element-wise
    Matrix dot(const Matrix& other) const;

    void print() const;

private:
    int rows;
    int cols;
    std::vector<float> data;
};

#endif // MATRIX_H
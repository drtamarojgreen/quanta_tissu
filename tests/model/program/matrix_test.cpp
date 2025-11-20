#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

// Helper to print test results
void check(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[  PASSED  ] " << test_name << std::endl;
    } else {
        std::cout << "[  FAILED  ] " << test_name << std::endl;
        throw std::runtime_error("Test failed: " + test_name);
    }
}

void test_initialization() {
    std::cout << "--- Testing Initialization ---" << std::endl;
    TissNum::Matrix m({2, 3});
    check(m.shape() == std::vector<int>({2, 3}), "Shape constructor");

    TissNum::Matrix zero_m({4, 5});
    bool all_zeros = true;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (zero_m.at({i, j}) != 0.0f) all_zeros = false;
        }
    }
    check(all_zeros, "Zeros initialization");

    TissNum::Matrix ones_m = TissNum::Matrix::ones({3, 2});
    bool all_ones = true;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            if (ones_m.at({i, j}) != 1.0f) all_ones = false;
        }
    }
    check(all_ones, "Ones method");
}

void test_reshape() {
    std::cout << "--- Testing Reshape ---" << std::endl;
    TissNum::Matrix m = TissNum::Matrix::random({2, 3});
    m.reshape({3, 2});
    check(m.shape() == std::vector<int>({3, 2}), "Valid reshape");

    try {
        m.reshape({4, 2});
        check(false, "Invalid reshape should throw");
    } catch (const std::invalid_argument& e) {
        check(true, "Invalid reshape should throw");
    }
}

void test_transpose() {
    std::cout << "--- Testing Transpose ---" << std::endl;
    TissNum::Matrix m = TissNum::Matrix::random({2, 3, 4});
    TissNum::Matrix transposed = m.transpose(0, 2);
    check(transposed.shape() == std::vector<int>({4, 3, 2}), "Valid transpose");

    try {
        m.transpose(0, 3);
        check(false, "Invalid transpose should throw");
    } catch (const std::out_of_range& e) {
        check(true, "Invalid transpose should throw");
    }
}

void test_matmul() {
    std::cout << "--- Testing Matmul ---" << std::endl;
    TissNum::Matrix a({2, 3});
    a.at({0, 0}) = 1; a.at({0, 1}) = 2; a.at({0, 2}) = 3;
    a.at({1, 0}) = 4; a.at({1, 1}) = 5; a.at({1, 2}) = 6;

    TissNum::Matrix b({3, 2});
    b.at({0, 0}) = 7; b.at({0, 1}) = 8;
    b.at({1, 0}) = 9; b.at({1, 1}) = 10;
    b.at({2, 0}) = 11; b.at({2, 1}) = 12;

    TissNum::Matrix c = TissNum::Matrix::matmul(a, b);
    check(c.shape() == std::vector<int>({2, 2}), "Matmul shape");
    check(c.at({0, 0}) == 58, "Matmul value [0,0]");
    check(c.at({0, 1}) == 64, "Matmul value [0,1]");
    check(c.at({1, 0}) == 139, "Matmul value [1,0]");
    check(c.at({1, 1}) == 154, "Matmul value [1,1]");

    TissNum::Matrix d({2, 2});
    try {
        TissNum::Matrix::matmul(a, d);
        check(false, "Invalid matmul should throw");
    } catch (const std::invalid_argument& e) {
        check(true, "Invalid matmul should throw");
    }
}

void test_element_wise_ops() {
    std::cout << "--- Testing Element-wise Operations ---" << std::endl;
    TissNum::Matrix a = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix b = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix c = a + b;
    check(c.at({0, 0}) == 2.0f, "Element-wise addition");
}

void test_scalar_ops() {
    std::cout << "--- Testing Scalar Operations ---" << std::endl;
    TissNum::Matrix a = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix b = a + 1.0f;
    check(b.at({0, 0}) == 2.0f, "Scalar addition");

    TissNum::Matrix d = b / 2.0f;
    check(d.at({0, 0}) == 1.0f, "Scalar division");
}

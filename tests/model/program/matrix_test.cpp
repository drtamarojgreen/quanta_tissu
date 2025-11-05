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
    check(m.get_shape() == std::vector<size_t>({2, 3}), "Shape constructor");
    check(m.rows() == 2, "Rows check");
    check(m.cols() == 3, "Cols check");

    TissNum::Matrix zero_m = TissNum::Matrix::zeros({4, 5});
    bool all_zeros = true;
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            if (zero_m({i, j}) != 0.0f) all_zeros = false;
        }
    }
    check(all_zeros, "Zeros method");

    TissNum::Matrix ones_m = TissNum::Matrix::ones({3, 2});
    bool all_ones = true;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            if (ones_m({i, j}) != 1.0f) all_ones = false;
        }
    }
    check(all_ones, "Ones method");
}

void test_reshape() {
    std::cout << "--- Testing Reshape ---" << std::endl;
    TissNum::Matrix m = TissNum::Matrix::random({2, 3});
    TissNum::Matrix reshaped = m.reshape({3, 2});
    check(reshaped.get_shape() == std::vector<size_t>({3, 2}), "Valid reshape");

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
    check(transposed.get_shape() == std::vector<size_t>({4, 3, 2}), "Valid transpose");

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
    a({0, 0}) = 1; a({0, 1}) = 2; a({0, 2}) = 3;
    a({1, 0}) = 4; a({1, 1}) = 5; a({1, 2}) = 6;

    TissNum::Matrix b({3, 2});
    b({0, 0}) = 7; b({0, 1}) = 8;
    b({1, 0}) = 9; b({1, 1}) = 10;
    b({2, 0}) = 11; b({2, 1}) = 12;

    TissNum::Matrix c = TissNum::Matrix::matmul(a, b);
    check(c.get_shape() == std::vector<size_t>({2, 2}), "Matmul shape");
    check(c({0, 0}) == 58, "Matmul value [0,0]");
    check(c({0, 1}) == 64, "Matmul value [0,1]");
    check(c({1, 0}) == 139, "Matmul value [1,0]");
    check(c({1, 1}) == 154, "Matmul value [1,1]");

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
    check(c({0, 0}) == 2.0f, "Element-wise addition");

    TissNum::Matrix d = a - b;
    check(d({0, 0}) == 0.0f, "Element-wise subtraction");

    TissNum::Matrix e = a * b;
    check(e({0, 0}) == 1.0f, "Element-wise multiplication");

    TissNum::Matrix f = a / b;
    check(f({0, 0}) == 1.0f, "Element-wise division");
}

void test_scalar_ops() {
    std::cout << "--- Testing Scalar Operations ---" << std::endl;
    TissNum::Matrix a = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix b = a + 1.0f;
    check(b({0, 0}) == 2.0f, "Scalar addition");

    TissNum::Matrix c = a - 1.0f;
    check(c({0, 0}) == 0.0f, "Scalar subtraction");

    TissNum::Matrix d = a * 2.0f;
    check(d({0, 0}) == 2.0f, "Scalar multiplication");

    TissNum::Matrix e = a / 2.0f;
    check(e({0, 0}) == 0.5f, "Scalar division");
}

void test_statistical_ops() {
    std::cout << "--- Testing Statistical Operations ---" << std::endl;
    TissNum::Matrix m({2, 3});
    m({0, 0}) = 1; m({0, 1}) = 2; m({0, 2}) = 3;
    m({1, 0}) = 4; m({1, 1}) = 5; m({1, 2}) = 6;

    TissNum::Matrix mean = m.mean(1);
    check(mean.get_shape() == std::vector<size_t>({2, 1}), "Mean shape");
    check(mean({0, 0}) == 2.0f, "Mean value [0,0]");
    check(mean({1, 0}) == 5.0f, "Mean value [1,0]");

    TissNum::Matrix max = m.max(1);
    check(max.get_shape() == std::vector<size_t>({2, 1}), "Max shape");
    check(max({0, 0}) == 3.0f, "Max value [0,0]");
    check(max({1, 0}) == 6.0f, "Max value [1,0]");
}

int main() {
    try {
        test_initialization();
        test_reshape();
        test_transpose();
        test_matmul();
        test_element_wise_ops();
        test_scalar_ops();
        test_statistical_ops();
        std::cout << "\nAll Matrix tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nMatrix tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
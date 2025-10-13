#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <stdexcept>

using namespace TissNum;

void test_matrix_concatenate() {
    std::cout << "=== Testing Matrix Concatenate ===" << std::endl;

    // Test case 1: Concatenate along rows (axis 0)
    Matrix a1(2, 2);
    a1(0, 0) = 1.0f; a1(0, 1) = 2.0f;
    a1(1, 0) = 3.0f; a1(1, 1) = 4.0f;

    Matrix b1(1, 2);
    b1(0, 0) = 5.0f; b1(0, 1) = 6.0f;

    Matrix c1 = Matrix::concatenate(a1, b1, 0);
    std::cout << "Concatenated along rows (expected 3x2):\n";
    // Expected: [[1, 2], [3, 4], [5, 6]]
    if (c1.rows() == 3 && c1.cols() == 2 &&
        c1(0,0) == 1.0f && c1(0,1) == 2.0f &&
        c1(1,0) == 3.0f && c1(1,1) == 4.0f &&
        c1(2,0) == 5.0f && c1(2,1) == 6.0f) {
        std::cout << "  Test Case 1 Passed\n";
    } else {
        std::cout << "  Test Case 1 FAILED\n";
        throw std::runtime_error("Matrix concatenate (axis 0) failed.");
    }

    // Test case 2: Concatenate along columns (axis 1)
    Matrix a2(2, 1);
    a2(0, 0) = 1.0f;
    a2(1, 0) = 2.0f;

    Matrix b2(2, 2);
    b2(0, 0) = 3.0f; b2(0, 1) = 4.0f;
    b2(1, 0) = 5.0f; b2(1, 1) = 6.0f;

    Matrix c2 = Matrix::concatenate(a2, b2, 1);
    std::cout << "Concatenated along columns (expected 2x3):\n";
    // Expected: [[1, 3, 4], [2, 5, 6]]
    if (c2.rows() == 2 && c2.cols() == 3 &&
        c2(0,0) == 1.0f && c2(0,1) == 3.0f && c2(0,2) == 4.0f &&
        c2(1,0) == 2.0f && c2(1,1) == 5.0f && c2(1,2) == 6.0f) {
        std::cout << "  Test Case 2 Passed\n";
    } else {
        std::cout << "  Test Case 2 FAILED\n";
        throw std::runtime_error("Matrix concatenate (axis 1) failed.");
    }

    // Test case 3: Mismatched dimensions (should throw exception)
    std::cout << "Testing mismatched dimensions (expected exception):\n";
    bool caught_exception = false;
    try {
        Matrix a3(2, 2);
        Matrix b3(1, 3); // Mismatched columns for axis 0
        Matrix::concatenate(a3, b3, 0);
    } catch (const std::invalid_argument& e) {
        std::cout << "  Caught expected exception: " << e.what() << "\n";
        caught_exception = true;
    }
    if (!caught_exception) {
        std::cout << "  Test Case 3 FAILED: No exception caught for mismatched columns.\n";
        throw std::runtime_error("Mismatched columns test failed.");
    }

    caught_exception = false;
    try {
        Matrix a4(2, 2);
        Matrix b4(3, 1); // Mismatched rows for axis 1
        Matrix::concatenate(a4, b4, 1);
    } catch (const std::invalid_argument& e) {
        std::cout << "  Caught expected exception: " << e.what() << "\n";
        caught_exception = true;
    }
    if (!caught_exception) {
        std::cout << "  Test Case 4 FAILED: No exception caught for mismatched rows.\n";
        throw std::runtime_error("Mismatched rows test failed.");
    }

    std::cout << "Matrix concatenate tests completed successfully." << std::endl << std::endl;
}

int main() {
    try {
        test_matrix_concatenate();
        std::cout << "All Matrix tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Matrix tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

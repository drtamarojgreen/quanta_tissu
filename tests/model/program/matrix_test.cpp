#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

// Helper to print test results
void check(bool condition, const std::string& test_name) {
    if (!condition) {
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
    } catch (const std::runtime_error& e) {
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

void test_transpose_data_permutation() {
    std::cout << "--- Testing Transpose Data Permutation ---" << std::endl;
    TissNum::Matrix m({2, 3});
    m({0, 0}) = 1.0f; m({0, 1}) = 2.0f; m({0, 2}) = 3.0f;
    m({1, 0}) = 4.0f; m({1, 1}) = 5.0f; m({1, 2}) = 6.0f;

    std::cout << "Original matrix data: ";
    const float* original_data_ptr = m.get_data();
    for(size_t i = 0; i < m.data_size(); ++i) {
        std::cout << original_data_ptr[i] << " ";
    }
    std::cout << std::endl;

    TissNum::Matrix transposed = m.transpose(0, 1);

    std::cout << "Transposed matrix data: ";
    const float* transposed_data_ptr = transposed.get_data();
    for(size_t i = 0; i < transposed.data_size(); ++i) {
        std::cout << transposed_data_ptr[i] << " ";
    }
    std::cout << std::endl;

    std::vector<float> expected_data = {1.0f, 4.0f, 2.0f, 5.0f, 3.0f, 6.0f};
    std::vector<float> actual_data(transposed_data_ptr, transposed_data_ptr + transposed.data_size());
    check(actual_data == expected_data, "Transpose data permutation");
}

void test_concatenate_data_permutation() {
    std::cout << "--- Testing Concatenate Data Permutation ---" << std::endl;
    TissNum::Matrix a({1, 2, 2});
    a({0, 0, 0}) = 1.0f; a({0, 0, 1}) = 2.0f;
    a({0, 1, 0}) = 3.0f; a({0, 1, 1}) = 4.0f;

    TissNum::Matrix b({1, 2, 3});
    b({0, 0, 0}) = 5.0f; b({0, 0, 1}) = 6.0f; b({0, 0, 2}) = 7.0f;
    b({0, 1, 0}) = 8.0f; b({0, 1, 1}) = 9.0f; b({0, 1, 2}) = 10.0f;

    TissNum::Matrix c = TissNum::Matrix::concatenate(a, b, 2);

    std::cout << "Concatenated matrix data: ";
    const float* c_data_ptr = c.get_data();
    for(size_t i = 0; i < c.data_size(); ++i) {
        std::cout << c_data_ptr[i] << " ";
    }
    std::cout << std::endl;

    std::vector<float> expected_data = {1.0f, 2.0f, 5.0f, 6.0f, 7.0f, 3.0f, 4.0f, 8.0f, 9.0f, 10.0f};
    std::vector<float> actual_data(c_data_ptr, c_data_ptr + c.data_size());
    check(c.get_shape() == std::vector<size_t>({1, 2, 5}), "Concatenate shape");
    check(actual_data == expected_data, "Concatenate data permutation");
}

void test_transpose_4d() {
    std::cout << "--- Testing Transpose 4D ---" << std::endl;
    TissNum::Matrix m({1, 2, 2, 3});
    // Data: 1-12
    float* m_data = m.get_data();
    for(size_t i=0; i<12; ++i) m_data[i] = i+1;

    TissNum::Matrix transposed = m.transpose(1, 2);
    check(transposed.get_shape() == std::vector<size_t>({1, 2, 2, 3}), "Transpose 4D shape");

    std::vector<float> expected_data = {1, 2, 3, 7, 8, 9, 4, 5, 6, 10, 11, 12};
    std::vector<float> actual_data(transposed.get_data(), transposed.get_data() + transposed.data_size());
    check(actual_data == expected_data, "Transpose 4D data permutation");
}

void test_concatenate_4d() {
    std::cout << "--- Testing Concatenate 4D ---" << std::endl;
    TissNum::Matrix a({1, 2, 1, 3});
    float* a_data = a.get_data();
    for(size_t i=0; i<6; ++i) a_data[i] = i+1;
    TissNum::Matrix b({1, 2, 1, 3});
    float* b_data = b.get_data();
    for(size_t i=0; i<6; ++i) b_data[i] = i+7;

    TissNum::Matrix c = TissNum::Matrix::concatenate(a, b, 2);
    check(c.get_shape() == std::vector<size_t>({1, 2, 2, 3}), "Concatenate 4D shape");

    std::vector<float> expected_data = {1, 2, 3, 7, 8, 9, 4, 5, 6, 10, 11, 12};
    std::vector<float> actual_data(c.get_data(), c.get_data() + c.data_size());
    check(actual_data == expected_data, "Concatenate 4D data permutation");
}

void test_attention_sequence() {
    std::cout << "--- Testing Attention Sequence ---" << std::endl;
    TissNum::Matrix x({1, 10, 16});
    TissNum::Matrix w = TissNum::Matrix::random({16, 16});

    TissNum::Matrix proj = TissNum::Matrix::matmul(x, w);
    check(proj.get_shape() == std::vector<size_t>({1, 10, 16}), "Attention sequence: matmul shape");

    TissNum::Matrix reshaped = proj.reshape({1, 10, 4, 4});
    check(reshaped.get_shape() == std::vector<size_t>({1, 10, 4, 4}), "Attention sequence: reshape shape");

    TissNum::Matrix transposed = reshaped.transpose(1, 2);
    check(transposed.get_shape() == std::vector<size_t>({1, 4, 10, 4}), "Attention sequence: transpose shape");

    TissNum::Matrix transposed_t = transposed.transpose(2, 3);
    TissNum::Matrix result = TissNum::Matrix::batch_matmul(transposed, transposed_t);
    check(result.get_shape() == std::vector<size_t>({1, 4, 10, 10}), "Attention sequence: batch_matmul shape");
}

void test_transpose_edge_cases() {
    std::cout << "--- Testing Transpose Edge Cases ---" << std::endl;
    TissNum::Matrix m1({1, 10});
    TissNum::Matrix t1 = m1.transpose(0, 1);
    check(t1.get_shape() == std::vector<size_t>({10, 1}), "Transpose with dim size 1");

    TissNum::Matrix m2({0, 10});
    TissNum::Matrix t2 = m2.transpose(0, 1);
    check(t2.get_shape() == std::vector<size_t>({10, 0}), "Transpose with dim size 0");

    TissNum::Matrix m3({5, 5});
    TissNum::Matrix t3 = m3.transpose(0, 0);
    check(t3.get_shape() == m3.get_shape(), "Transpose with same dims");
}

void test_concatenate_edge_cases() {
    std::cout << "--- Testing Concatenate Edge Cases ---" << std::endl;
    TissNum::Matrix a({1, 2, 2});
    TissNum::Matrix b({1, 2, 0});
    TissNum::Matrix c = TissNum::Matrix::concatenate(a, b, 2);
    check(c.get_shape() == std::vector<size_t>({1, 2, 2}), "Concatenate with empty matrix");

    try {
        TissNum::Matrix d({1, 2});
        TissNum::Matrix::concatenate(a, d, 2);
        check(false, "Concatenate with different num dims should throw");
    } catch (const std::invalid_argument& e) {
        check(true, "Concatenate with different num dims should throw");
    }
}

void test_reshape_edge_cases() {
    std::cout << "--- Testing Reshape Edge Cases ---" << std::endl;
    TissNum::Matrix m({2, 3, 4});
    TissNum::Matrix r1 = m.reshape({24});
    check(r1.get_shape() == std::vector<size_t>({24}), "Reshape to different num dims");

    TissNum::Matrix r2 = m.reshape({2, 12});
    check(r2.get_shape() == std::vector<size_t>({2, 12}), "Reshape to different shape");
}

void test_matmul_edge_cases() {
    std::cout << "--- Testing Matmul Edge Cases ---" << std::endl;
    TissNum::Matrix a({2, 0});
    TissNum::Matrix b({0, 3});
    TissNum::Matrix c = TissNum::Matrix::matmul(a, b);
    check(c.get_shape() == std::vector<size_t>({2, 3}), "Matmul with zero dim");

    try {
        TissNum::Matrix d({2, 3});
        TissNum::Matrix e({4, 2});
        TissNum::Matrix::matmul(d, e);
        check(false, "Matmul with incompatible inner dims should throw");
    } catch (const std::invalid_argument& e) {
        check(true, "Matmul with incompatible inner dims should throw");
    }
}

void test_operator_edge_cases() {
    std::cout << "--- Testing Operator Edge Cases ---" << std::endl;
    TissNum::Matrix m({2, 3});
    try {
        m({0, 0, 0});
        check(false, "Access with wrong num indices should throw");
    } catch (const std::out_of_range& e) {
        check(true, "Access with wrong num indices should throw");
    }

    try {
        m({2, 0});
        check(false, "Access with out of bounds index should throw");
    } catch (const std::out_of_range& e) {
        check(true, "Access with out of bounds index should throw");
    }
}

void test_statistical_ops_edge_cases() {
    std::cout << "--- Testing Statistical Ops Edge Cases ---" << std::endl;
    TissNum::Matrix m({0, 3});
    TissNum::Matrix mean = m.mean(1);
    check(mean.get_shape() == std::vector<size_t>({0, 1}), "Mean of empty matrix");
}

void test_element_wise_ops_edge_cases() {
    std::cout << "--- Testing Element-wise Ops Edge Cases ---" << std::endl;
    TissNum::Matrix a({2, 3});
    TissNum::Matrix b({3, 2});
    try {
        TissNum::Matrix c = a + b;
        check(false, "Element-wise op with incompatible shapes should throw");
    } catch (const std::invalid_argument& e) {
        check(true, "Element-wise op with incompatible shapes should throw");
    }
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
        test_transpose_data_permutation();
        test_concatenate_data_permutation();
        test_transpose_4d();
        test_concatenate_4d();
        test_attention_sequence();
        test_transpose_edge_cases();
        test_concatenate_edge_cases();
        test_reshape_edge_cases();
        test_matmul_edge_cases();
        test_operator_edge_cases();
        test_statistical_ops_edge_cases();
        test_element_wise_ops_edge_cases();
        std::cout << "\nAll Matrix tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nMatrix tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
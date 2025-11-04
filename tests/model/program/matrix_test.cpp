#include "gtest/gtest.h"
#include "core/matrix.h"
#include <vector>
#include <cmath>

TEST(MatrixTest, Initialization) {
    TissNum::Matrix m({2, 3});
    EXPECT_EQ(m.get_shape(), std::vector<size_t>({2, 3}));
    EXPECT_EQ(m.rows(), 2);
    EXPECT_EQ(m.cols(), 3);

    TissNum::Matrix zero_m = TissNum::Matrix::zeros({4, 5});
    EXPECT_EQ(zero_m.get_shape(), std::vector<size_t>({4, 5}));
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            EXPECT_EQ(zero_m({i, j}), 0.0f);
        }
    }

    TissNum::Matrix ones_m = TissNum::Matrix::ones({3, 2});
    EXPECT_EQ(ones_m.get_shape(), std::vector<size_t>({3, 2}));
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            EXPECT_EQ(ones_m({i, j}), 1.0f);
        }
    }
}

TEST(MatrixTest, Reshape) {
    TissNum::Matrix m = TissNum::Matrix::random({2, 3});
    TissNum::Matrix reshaped = m.reshape({3, 2});
    EXPECT_EQ(reshaped.get_shape(), std::vector<size_t>({3, 2}));

    EXPECT_THROW(m.reshape({4, 2}), std::invalid_argument);
}

TEST(MatrixTest, Transpose) {
    TissNum::Matrix m = TissNum::Matrix::random({2, 3, 4});
    TissNum::Matrix transposed = m.transpose(0, 2);
    EXPECT_EQ(transposed.get_shape(), std::vector<size_t>({4, 3, 2}));

    EXPECT_THROW(m.transpose(0, 3), std::out_of_range);
}

TEST(MatrixTest, Matmul) {
    TissNum::Matrix a({2, 3});
    a({0, 0}) = 1; a({0, 1}) = 2; a({0, 2}) = 3;
    a({1, 0}) = 4; a({1, 1}) = 5; a({1, 2}) = 6;

    TissNum::Matrix b({3, 2});
    b({0, 0}) = 7; b({0, 1}) = 8;
    b({1, 0}) = 9; b({1, 1}) = 10;
    b({2, 0}) = 11; b({2, 1}) = 12;

    TissNum::Matrix c = TissNum::Matrix::matmul(a, b);
    EXPECT_EQ(c.get_shape(), std::vector<size_t>({2, 2}));
    EXPECT_EQ(c({0, 0}), 58);
    EXPECT_EQ(c({0, 1}), 64);
    EXPECT_EQ(c({1, 0}), 139);
    EXPECT_EQ(c({1, 1}), 154);

    TissNum::Matrix d({2, 2});
    EXPECT_THROW(TissNum::Matrix::matmul(a, d), std::invalid_argument);
}

TEST(MatrixTest, ElementWiseOperations) {
    TissNum::Matrix a = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix b = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix c = a + b;
    EXPECT_EQ(c({0, 0}), 2.0f);

    TissNum::Matrix d = a - b;
    EXPECT_EQ(d({0, 0}), 0.0f);

    TissNum::Matrix e = a * b;
    EXPECT_EQ(e({0, 0}), 1.0f);

    TissNum::Matrix f = a / b;
    EXPECT_EQ(f({0, 0}), 1.0f);
}

TEST(MatrixTest, ScalarOperations) {
    TissNum::Matrix a = TissNum::Matrix::ones({2, 2});
    TissNum::Matrix b = a + 1.0f;
    EXPECT_EQ(b({0, 0}), 2.0f);

    TissNum::Matrix c = a - 1.0f;
    EXPECT_EQ(c({0, 0}), 0.0f);

    TissNum::Matrix d = a * 2.0f;
    EXPECT_EQ(d({0, 0}), 2.0f);

    TissNum::Matrix e = a / 2.0f;
    EXPECT_EQ(e({0, 0}), 0.5f);
}

TEST(MatrixTest, StatisticalOperations) {
    TissNum::Matrix m({2, 3});
    m({0, 0}) = 1; m({0, 1}) = 2; m({0, 2}) = 3;
    m({1, 0}) = 4; m({1, 1}) = 5; m({1, 2}) = 6;

    TissNum::Matrix mean = m.mean(1);
    EXPECT_EQ(mean.get_shape(), std::vector<size_t>({2, 1}));
    EXPECT_EQ(mean({0, 0}), 2.0f);
    EXPECT_EQ(mean({1, 0}), 5.0f);

    TissNum::Matrix max = m.max(1);
    EXPECT_EQ(max.get_shape(), std::vector<size_t>({2, 1}));
    EXPECT_EQ(max({0, 0}), 3.0f);
    EXPECT_EQ(max({1, 0}), 6.0f);

    TissNum::Matrix exp = TissNum::Matrix::exp(m);
    EXPECT_EQ(exp.get_shape(), m.get_shape());
    std::cout << "Matrix concatenate tests completed successfully." << std::endl << std::endl;
}

void test_matrix_multidim_ops() {
    std::cout << "=== Testing Matrix Multi-dimensional Operations ===" << std::endl;

    TissNum::Matrix m({2, 2, 3});
    m({0, 0, 0}) = 1; m({0, 0, 1}) = 2; m({0, 0, 2}) = 3;
    m({0, 1, 0}) = 4; m({0, 1, 1}) = 5; m({0, 1, 2}) = 6;
    m({1, 0, 0}) = 7; m({1, 0, 1}) = 8; m({1, 0, 2}) = 9;
    m({1, 1, 0}) = 10; m({1, 1, 1}) = 11; m({1, 1, 2}) = 12;

    TissNum::Matrix sum0 = m.sum(0);
    EXPECT_EQ(sum0.get_shape(), std::vector<size_t>({1, 2, 3}));
    EXPECT_EQ(sum0({0, 0, 0}), 8);

    TissNum::Matrix sum1 = m.sum(1);
    EXPECT_EQ(sum1.get_shape(), std::vector<size_t>({2, 1, 3}));
    EXPECT_EQ(sum1({0, 0, 0}), 5);

    TissNum::Matrix sum2 = m.sum(2);
    EXPECT_EQ(sum2.get_shape(), std::vector<size_t>({2, 2, 1}));
    EXPECT_EQ(sum2({0, 0, 0}), 6);

        std::cout << "Matrix multi-dimensional operations tests completed successfully." << std::endl << std::endl;

    }

    

    void test_matrix_multidim_stats() {

        std::cout << "=== Testing Matrix Multi-dimensional Stats ===" << std::endl;

    

        TissNum::Matrix m({2, 2, 3});

        m({0, 0, 0}) = 1; m({0, 0, 1}) = 2; m({0, 0, 2}) = 3;

        m({0, 1, 0}) = 4; m({0, 1, 1}) = 5; m({0, 1, 2}) = 6;

        m({1, 0, 0}) = 7; m({1, 0, 1}) = 8; m({1, 0, 2}) = 9;

        m({1, 1, 0}) = 10; m({1, 1, 1}) = 11; m({1, 1, 2}) = 12;

    

        TissNum::Matrix mean0 = m.mean(0);

        EXPECT_EQ(mean0.get_shape(), std::vector<size_t>({1, 2, 3}));

        EXPECT_EQ(mean0({0, 0, 0}), 4);

    

        TissNum::Matrix mean1 = m.mean(1);

        EXPECT_EQ(mean1.get_shape(), std::vector<size_t>({2, 1, 3}));

        EXPECT_EQ(mean1({0, 0, 0}), 2.5);

    

        TissNum::Matrix mean2 = m.mean(2);

        EXPECT_EQ(mean2.get_shape(), std::vector<size_t>({2, 2, 1}));

        EXPECT_EQ(mean2({0, 0, 0}), 2);

    

        TissNum::Matrix max0 = m.max(0);

        EXPECT_EQ(max0.get_shape(), std::vector<size_t>({1, 2, 3}));

        EXPECT_EQ(max0({0, 0, 0}), 7);

    

        TissNum::Matrix max1 = m.max(1);

        EXPECT_EQ(max1.get_shape(), std::vector<size_t>({2, 1, 3}));

        EXPECT_EQ(max1({0, 0, 0}), 4);

    

        TissNum::Matrix max2 = m.max(2);

        EXPECT_EQ(max2.get_shape(), std::vector<size_t>({2, 2, 1}));

        EXPECT_EQ(max2({0, 0, 0}), 3);

    

        std::cout << "Matrix multi-dimensional stats tests completed successfully." << std::endl << std::endl;

    }

    

    int main() {

        try {

            test_matrix_concatenate();

            test_matrix_multidim_ops();

            test_matrix_multidim_stats();

            std::cout << "All Matrix tests passed!" << std::endl;

            return 0;

        } catch (const std::exception& e) {

            std::cerr << "Matrix tests failed with exception: " << e.what() << std::endl;

            return 1;

        }

    }

    
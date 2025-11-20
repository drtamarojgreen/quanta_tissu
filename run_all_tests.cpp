#include <iostream>

// Include all the refactored .cpp files directly
#include "quanta_tissu/tisslm/program/core/matrix.cpp"
#include "quanta_tissu/tisslm/program/core/parameter.cpp"
#include "quanta_tissu/tisslm/program/core/layernorm.cpp"
#include "quanta_tissu/tisslm/program/core/dropout.cpp"
#include "quanta_tissu/tisslm/program/core/feedforward.cpp"
#include "quanta_tissu/tisslm/program/core/configurableattention.cpp"
#include "quanta_tissu/tisslm/program/core/transformerblock.cpp"
#include "quanta_tissu/tisslm/program/core/positionalencoding.cpp"
#include "quanta_tissu/tisslm/program/core/embedding.cpp"

// Include the refactored matrix test cases
#include "tests/model/program/matrix_test.cpp"

int main() {
    try {
        test_initialization();
        test_reshape();
        test_transpose();
        test_matmul();
        test_element_wise_ops();
        test_scalar_ops();
        std::cout << "\nAll Matrix tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nMatrix tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
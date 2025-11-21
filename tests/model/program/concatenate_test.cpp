#include <iostream>
#include <cassert>
#include <vector>
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"

using namespace TissNum;

void test_concatenate_with_broadcasting() {
    std::cout << "--- Testing Concatenate with Broadcasting ---" << std::endl;

    // Test Case: Replicating the MQA KV Cache bug
    // A multi-headed cache tensor
    Matrix cache({1, 8, 5, 4}); 
    for(size_t h = 0; h < 8; ++h) {
        for(size_t s = 0; s < 5; ++s) {
            for(size_t d = 0; d < 4; ++d) {
                cache({0, h, s, d}) = (h + 1) * 100 + s; 
            }
        }
    }

    // A single-headed new token tensor
    Matrix new_token({1, 1, 1, 4});
    for(size_t d = 0; d < 4; ++d) {
        new_token({0, 0, 0, d}) = 999;
    }

    // Concatenate along the sequence length axis (axis 2)
    Matrix result = Matrix::concatenate(cache, new_token, 2);

    // 1. Check the shape
    std::vector<size_t> expected_shape = {1, 8, 6, 4};
    assert(result.get_shape() == expected_shape);
    std::cout << "[  PASSED  ] Concatenated shape is correct." << std::endl;

    // 2. Check the data integrity and broadcasting
    bool broadcast_correct = true;
    for(size_t h = 0; h < 8; ++h) {
        // Check that the last token in the sequence for each head is the new token data
        for(size_t d = 0; d < 4; ++d) {
            if (result({0, h, 5, d}) != 999) {
                broadcast_correct = false;
                std::cerr << "[  FAILED  ] Data mismatch at head " << h << ", dim " << d << ". Expected 999, got " << result({0, h, 5, d}) << std::endl;
                break;
            }
        }
        if (!broadcast_correct) break;
    }
    
    if (broadcast_correct) {
        std::cout << "[  PASSED  ] New token data was correctly broadcast to all heads." << std::endl;
    } else {
        std::cerr << "[  FAILED  ] Broadcasting of new token data failed." << std::endl;
    }
    
    assert(broadcast_correct);
}

void test_concatenate_with_broadcast_non_adjacent() {
    std::cout << "\n--- Testing Concatenate with Non-Adjacent Broadcasting ---" << std::endl;

    Matrix a({8, 5, 1, 4});
    for(size_t i = 0; i < 8; ++i) a({i, 0, 0, 0}) = i;

    Matrix b({1, 5, 1, 4});
    b({0, 0, 0, 0}) = 99;

    Matrix result = Matrix::concatenate(a, b, 0); // Concatenate along the first axis

    std::vector<size_t> expected_shape = {9, 5, 1, 4};
    assert(result.get_shape() == expected_shape);
    std::cout << "[  PASSED  ] Concatenated shape is correct." << std::endl;

    bool data_correct = true;
    if (result({8, 0, 0, 0}) != 99) {
        data_correct = false;
        std::cerr << "[  FAILED  ] Data mismatch. Expected 99, got " << result({8, 0, 0, 0}) << std::endl;
    }

    if (data_correct) {
        std::cout << "[  PASSED  ] Data is correct after non-adjacent broadcast." << std::endl;
    }
    assert(data_correct);
}

int main() {
    try {
        test_concatenate_with_broadcasting();
        test_concatenate_with_broadcast_non_adjacent();
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

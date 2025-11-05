#include "../../../quanta_tissu/tisslm/program/core/multiheadattention.h"
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

void test_lora_forward() {
    std::cout << "--- Testing LoRA Forward ---" << std::endl;
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in({1, 16});
    TissNum::Matrix k_in({1, 16});
    TissNum::Matrix v_in({1, 16});

    TissNum::Matrix output = mha.forward(q_in, k_in, v_in);

    check(output.get_shape() == std::vector<size_t>({1, 16}), "LoRA forward output shape");
}

void test_no_lora() {
    std::cout << "--- Testing No LoRA ---" << std::endl;
    TissNum::MultiHeadAttention mha(16, 4, 0);
    auto params = mha.parameters();
    bool lora_found = false;
    for (auto p : params) {
        if (p->name().find("lora") != std::string::npos) {
            lora_found = true;
            break;
        }
    }
    check(!lora_found, "No LoRA parameters when rank is 0");
}

int main() {
    try {
        test_lora_forward();
        test_no_lora();
        std::cout << "\nAll LoRA tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nLoRA tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

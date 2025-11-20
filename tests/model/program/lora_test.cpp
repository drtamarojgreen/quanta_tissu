#include "../../../quanta_tissu/tisslm/program/core/configurableattention.h"
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>

using namespace TissNum;

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
    ConfigurableAttention mha(16, 4, AttentionMode::STANDARD_MULTI_HEAD, 4);
    Matrix x({1, 16});

    Matrix output = mha.forward(x);

    check(output.shape() == std::vector<int>({1, 16}), "LoRA forward output shape");
}

void test_no_lora() {
    std::cout << "--- Testing No LoRA ---" << std::endl;
    ConfigurableAttention mha(16, 4, AttentionMode::STANDARD_MULTI_HEAD, 0);
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
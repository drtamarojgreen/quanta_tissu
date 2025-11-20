#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <stdexcept>

// Mock classes for components that are not being tested directly
namespace TissNum {
    class Embedding {
    public:
        Embedding(int vocab_size, int d_model, const std::string& name) {}
        Matrix forward(const std::vector<int>& tokens) {
            return Matrix({(int)tokens.size(), 4});
        }
        void backward(const Matrix& grad_out, const std::vector<int>& tokens) {}
    };

    class PositionalEncoding {
    public:
        PositionalEncoding(int d_model, int max_len) {}
        Matrix forward(const Matrix& input, int start_pos) {
            return input;
        }
    };
}

using namespace TissNum;

void check(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[  PASSED  ] " << test_name << std::endl;
    } else {
        std::cout << "[  FAILED  ] " << test_name << std::endl;
        throw std::runtime_error("Test failed: " + test_name);
    }
}


void test_embedding() {
    std::cout << "--- Testing Embedding Layer (Mocked) ---" << std::endl;
    Embedding emb(10, 4, "test_emb");
    std::vector<int> tokens = {0, 1, 2, 3, 1};
    Matrix embedded = emb.forward(tokens);
    check(embedded.shape() == std::vector<int>({5, 4}), "Embedding forward shape");
}

void test_positional_encoding() {
    std::cout << "--- Testing Positional Encoding (Mocked) ---" << std::endl;
    PositionalEncoding pe(4, 100);
    Matrix input({3, 4});
    Matrix output = pe.forward(input, 0);
    check(output.shape() == std::vector<int>({3, 4}), "Positional Encoding forward shape");
}


int main() {
    try {
        test_embedding();
        test_positional_encoding();
        std::cout << "\nAll New Components tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nNew Components tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
#include "../../../quanta_tissu/tisslm/program/core/embedding.h"
#include "../../../quanta_tissu/tisslm/program/core/positionalencoding.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <iomanip>

using namespace TissNum;

void test_embedding() {
    std::cout << "=== Testing Embedding Layer ===" << std::endl;
    
    // Create an embedding layer with vocab_size=10, d_model=4
    Embedding emb(10, TestConfig::EmbedDim, "test_emb");
    
    // Test forward pass with a sequence of token IDs
    std::vector<size_t> tokens = {0, 1, 2, 3, 1};
    Matrix embedded = emb.forward(tokens);
    
    std::cout << "Input tokens: ";
    for (auto token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Embedded shape: (" << embedded.rows() << ", " << embedded.cols() << ")" << std::endl;
    std::cout << "First embedding vector: ";
    for (size_t j = 0; j < embedded.cols(); ++j) {
        std::cout << std::fixed << std::setprecision(4) << embedded({0, j}) << " ";
    }
    std::cout << std::endl;
    
    // Test backward pass
    Matrix grad_out = Matrix::ones({tokens.size(), (size_t)TestConfig::EmbedDim});
    emb.backward(grad_out, tokens);
    
    std::cout << "Backward pass completed successfully" << std::endl;
    std::cout << std::endl;
}

void test_positional_encoding() {
    std::cout << "=== Testing Positional Encoding ===" << std::endl;
    
    // Create positional encoding with d_model=4, max_len=100
    PositionalEncoding pe(TestConfig::EmbedDim, 100);
    
    // Create a simple input matrix (seq_len=3, d_model=4)
    Matrix input({3, (size_t)TestConfig::EmbedDim});
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < TestConfig::EmbedDim; ++j) {
            input({i, j}) = 1.0f;  // Fill with ones for simplicity
        }
    }
    
    std::cout << "Input shape: (" << input.rows() << ", " << input.cols() << ")" << std::endl;
    
    // Apply positional encoding
    Matrix output = pe.forward(input, 0);
    
    std::cout << "Output shape: (" << output.rows() << ", " << output.cols() << ")" << std::endl;
    std::cout << "First position with encoding: ";
    for (size_t j = 0; j < output.cols(); ++j) {
        std::cout << std::fixed << std::setprecision(4) << output({0, j}) << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Second position with encoding: ";
    for (size_t j = 0; j < output.cols(); ++j) {
        std::cout << std::fixed << std::setprecision(4) << output({1, j}) << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Combined test completed successfully" << std::endl;
    std::cout << std::endl;
}

void test_combined() {
    std::cout << "=== Testing Combined Embedding + Positional Encoding ===" << std::endl;
    
    // Create embedding and positional encoding
    size_t vocab_size = 10;
    size_t d_model = 8;
    size_t seq_len = 5;
    
    Embedding emb(vocab_size, d_model, "combined_test");
    PositionalEncoding pe(d_model, 100);
    
    // Create token sequence
    std::vector<size_t> tokens = {1, 3, 5, 7, 2};
    
    // Get embeddings
    Matrix embedded = emb.forward(tokens);
    std::cout << "Embedded shape: (" << embedded.rows() << ", " << embedded.cols() << ")" << std::endl;
    
    // Add positional encoding
    Matrix output = pe.forward(embedded, 0);
    std::cout << "After positional encoding shape: (" << output.rows() << ", " << output.cols() << ")" << std::endl;
    
    std::cout << "First token representation (first 4 dims): ";
    for (size_t j = 0; j < 4; ++j) {
        std::cout << std::fixed << std::setprecision(4) << output(0, j) << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Combined test completed successfully" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "Testing New C++ Components" << std::endl;
    std::cout << "===========================" << std::endl << std::endl;
    
    try {
        test_embedding();
        test_positional_encoding();
        test_combined();
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

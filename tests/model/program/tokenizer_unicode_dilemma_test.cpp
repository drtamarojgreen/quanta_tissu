#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <string>
#include <vector>

using namespace TissLM::Tokenizer;

// Keep track of overall test success.
bool overall_success = true;

void run_dilemma_test(const std::string& test_name, const std::string& input_text) {
    Tokenizer tokenizer(TestConfig::TokenizerPath);
    std::vector<int> encoded_tokens = tokenizer.encode(input_text);
    std::string decoded_text = tokenizer.decode(encoded_tokens);

    // This is the core of the dilemma test: we expect the decoding to FAIL.
    // Success for this test means the decoded text does NOT match the original.
    if (input_text != decoded_text) {
        std::cout << "  [PASS] " << test_name << ": Tokenizer correctly failed to preserve Unicode." << std::endl;
        std::cout << "    Original: " << input_text << std::endl;
        std::cout << "    Decoded:  " << decoded_text << std::endl;
    } else {
        std::cout << "  [FAIL] " << test_name << ": Tokenizer unexpectedly preserved Unicode." << std::endl;
        overall_success = false;
    }
}

int main() {
    std::cout << "=== Testing Tokenizer Unicode Dilemma ===" << std::endl;
    std::cout << "This test suite demonstrates the tokenizer's failure to handle Unicode" << std::endl;
    std::cout << "due to its regex-based pre-tokenization logic." << std::endl;
    std::cout << "A 'PASS' indicates the tokenizer failed as expected, proving the dilemma." << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    // Test Case 1: Non-Latin Scripts
    std::string non_latin_text = "Привет, мир! (Russian)";
    run_dilemma_test("Non-Latin Script Test", non_latin_text);

    // Test Case 2: Emojis
    std::string emoji_text = "Testing emojis: 😊👍🚀";
    run_dilemma_test("Emoji Test", emoji_text);

    std::cout << "-------------------------------------------------" << std::endl;

    if (overall_success) {
        std::cout << "Conclusion: All tests passed. The Tokenizer's inability to handle" << std::endl;
        std::cout << "Unicode has been successfully demonstrated." << std::endl;
        return 0; // Success: The dilemma is proven.
    } else {
        std::cout << "Conclusion: Some tests failed, indicating the tokenizer's behavior" << std::endl;
        std::cout << "is not as expected. The dilemma is not fully reproduced." << std::endl;
        return 1; // Failure: The expected failure did not occur.
    }
}

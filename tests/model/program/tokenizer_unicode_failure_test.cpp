#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <string>
#include <vector>

using namespace TissLM::Tokenizer;

// Keep track of overall test success.
bool overall_success = true;

void run_failure_test(const std::string& test_name, const std::string& input_text) {
    Tokenizer tokenizer(TestConfig::TokenizerPath);
    std::vector<int> encoded_tokens = tokenizer.encode(input_text);
    std::string decoded_text = tokenizer.decode(encoded_tokens);

    // This test PASSES if the decoded text does NOT match the original, proving the bug.
    if (input_text != decoded_text) {
        std::cout << "  [PASS] " << test_name << ": Tokenizer correctly failed to preserve the string." << std::endl;
        std::cout << "    Original: " << input_text << std::endl;
        std::cout << "    Decoded:  " << decoded_text << std::endl;
    } else {
        std::cout << "  [FAIL] " << test_name << ": Tokenizer unexpectedly preserved the string." << std::endl;
        overall_success = false;
    }
}

int main() {
    std::cout << "=== Comprehensive Tokenizer Unicode Failure Test ===" << std::endl;
    std::cout << "This test suite demonstrates the tokenizer's inability to handle a wide" << std::endl;
    std::cout << "range of Unicode characters due to its regex-based pre-tokenization." << std::endl;
    std::cout << "A 'PASS' indicates the tokenizer failed as expected, proving the dilemma." << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;

    // Test Case 1: Accented Latin Characters
    run_failure_test("Accented Latin Characters", "éàçüö");

    // Test Case 2: Cyrillic Script
    run_failure_test("Cyrillic Script", "Привет, мир!");

    // Test Case 3: Japanese Script
    run_failure_test("Japanese Script", "こんにちは、世界！");

    // Test Case 4: Korean Script
    run_failure_test("Korean Script", "안녕하세요, 세계!");

    // Test Case 5: Arabic Script
    run_failure_test("Arabic Script", "مرحبا بالعالم");

    // Test Case 6: Hebrew Script
    run_failure_test("Hebrew Script", "שלום עולם");

    // Test Case 7: Symbols and Punctuation
    run_failure_test("Symbols and Punctuation", "Testing symbols: ©®™…–—");

    // Test Case 8: Emojis
    run_failure_test("Emojis", "Testing emojis: 😊👍🚀🎉");

    // Test Case 9: Mixed String
    run_failure_test("Mixed String", "Hello, Привет, こんにちは, 😊!");

    std::cout << "--------------------------------------------------------------------" << std::endl;

    if (overall_success) {
        std::cout << "Conclusion: All tests passed. The Tokenizer's inability to handle" << std::endl;
        std::cout << "a comprehensive set of Unicode characters has been demonstrated." << std::endl;
        return 0; // Success: The dilemma is proven.
    } else {
        std::cout << "Conclusion: One or more tests failed, indicating the tokenizer's behavior" << std::endl;
        std::cout << "is not as expected. The dilemma is not fully reproduced." << std::endl;
        return 1; // Failure: The expected failure did not occur.
    }
}

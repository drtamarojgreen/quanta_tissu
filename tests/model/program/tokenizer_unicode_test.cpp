#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <string>
#include <vector>
#include <utility>

using namespace TissLM::Tokenizer;

// Keep track of overall test success.
bool all_tests_passed = true;

// A single, clear test function that asserts for equality.
void run_unicode_test(const std::string& test_name, const std::string& input_text) {
    Tokenizer tokenizer(TestConfig::TokenizerPath);
    std::vector<int> encoded_tokens = tokenizer.encode(input_text);
    std::string decoded_text = tokenizer.decode(encoded_tokens);

    // This is a standard test: it PASSES if the output MATCHES the input.
    if (input_text == decoded_text) {
        std::cout << "  [PASS] " << test_name << std::endl;
    } else {
        std::cout << "  [FAIL] " << test_name << std::endl;
        std::cout << "    Original: " << input_text << std::endl;
        std::cout << "    Decoded:  " << decoded_text << std::endl;
        all_tests_passed = false;
    }
}

int main() {
    std::cout << "=== Comprehensive Tokenizer Unicode Test ===" << std::endl;
    std::cout << "This test suite evaluates the tokenizer's ability to handle Unicode." << std::endl;
    std::cout << "It is EXPECTED TO FAIL in the current environment due to a known bug." << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;

    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"Accented Latin", "La cigüeña tañe el ñandú al alimón: ¡qué follón!"},
        {"Cyrillic Script", "Привет, мир! Как дела?"},
        {"Japanese Script", "こんにちは、世界！"},
        {"Korean Script", "안녕하세요, 세계!"},
        {"Arabic Script", "مرحبا بالعالم"},
        {"Hebrew Script", "שלום עולם"},
        {"Greek Script", "Γειά σου Κόσμε"},
        {"Thai Script", "สวัสดีชาวโลก"},
        {"Symbols & Currency", "Testing: ©®™, €, £, ¥, …–—"},
        {"Diverse Emojis", "😊👍🚀🎉❤️😂🤔🔥"},
        {"Mixed Complex String", "UTF-8 test: Привет, €/£, こんにちは, 😊!"}
    };

    for (const auto& test_case : test_cases) {
        run_unicode_test(test_case.first, test_case.second);
    }

    std::cout << "--------------------------------------------------------------------" << std::endl;

    if (all_tests_passed) {
        std::cout << "Conclusion: All Unicode tests passed unexpectedly." << std::endl;
        return 0;
    } else {
        std::cout << "Conclusion: One or more Unicode tests failed as expected." << std::endl;
        std::cout << "This confirms the tokenizer's known bug with Unicode characters." << std::endl;
        return 1; // Return a failure code to the shell.
    }
}

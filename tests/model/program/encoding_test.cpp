#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <string>
#include <vector>

using namespace TissLM::Tokenizer;

int main() {
    try {
        std::cout << "=== Testing Text Encoding ===" << std::endl;

        // 1. Sample text with a variety of UTF-8 characters
        std::string original_text = "Hello, world! This is a test with some special characters: éàçüö, and some emojis: 😊👍🚀. Let's also include some non-latin scripts: Привет, мир! (Russian), こんにちは、世界！ (Japanese), and 안녕하세요, 세계! (Korean).";

        // 2. Create a tokenizer
        Tokenizer tokenizer(TestConfig::TokenizerPath);

        // 3. Encode the text
        std::vector<int> encoded_tokens = tokenizer.encode(original_text);

        // 4. Decode the tokens
        std::string decoded_text = tokenizer.decode(encoded_tokens);

        // 5. Assert that the decoded text is the same as the original
        if (original_text == decoded_text) {
            std::cout << "  [PASSED] UTF-8 text was encoded and decoded successfully without loss." << std::endl;
        } else {
            std::cout << "  [FAILED] Decoded text does not match the original text." << std::endl;
            std::cout << "    Original: " << original_text << std::endl;
            std::cout << "    Decoded:  " << decoded_text << std::endl;
            return 1;
        }

        std::cout << "Text encoding test completed successfully." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Text encoding test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

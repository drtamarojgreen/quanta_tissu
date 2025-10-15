#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include <iostream>
#include <vector>
#include <string>
#include <numeric>

void test_tokenizer() {
    std::cout << "=== Testing Tokenizer ===" << std::endl;

    // Assuming dummy_vocab.json and dummy_merges.txt are in the same directory as the test executable
    // The Tokenizer constructor expects a prefix, so we pass "dummy"
    Tokenizer tokenizer("../dummy"); 

    // Test get_vocab_size
    if (tokenizer.get_vocab_size() == 6) { // Based on dummy_vocab.json
        std::cout << "  get_vocab_size Passed\n";
    } else {
        std::cout << "  get_vocab_size FAILED (Expected 6, Got " << tokenizer.get_vocab_size() << ")\n";
        throw std::runtime_error("Tokenizer get_vocab_size failed.");
    }

    // Test encode
    std::string text_to_encode = "ab cd";
    std::vector<int> encoded_ids = tokenizer.encode(text_to_encode);
    std::vector<int> expected_ids = {4, 3, 5}; // 'ab' -> 4, ' ' -> 3 (assuming space is token 3), 'cd' -> 5

    if (encoded_ids == expected_ids) {
        std::cout << "  Encode Passed\n";
    } else {
        std::cout << "  Encode FAILED (Expected: ";
        for (int id : expected_ids) std::cout << id << " ";
        std::cout << ", Got: ";
        for (int id : encoded_ids) std::cout << id << " ";
        std::cout << ")\n";
        throw std::runtime_error("Tokenizer encode failed.");
    }

    // Test decode
    std::string decoded_text = tokenizer.decode(encoded_ids);
    std::string expected_text = "ab cd"; // Assuming space is handled correctly

    if (decoded_text == expected_text) {
        std::cout << "  Decode Passed\n";
    } else {
        std::cout << "  Decode FAILED (Expected: \"" << expected_text << ", Got: \"" << decoded_text << ")\n";
        throw std::runtime_error("Tokenizer decode failed.");
    }

    std::cout << "Tokenizer tests completed successfully." << std::endl << std::endl;
}

void test_tokenizer_training() {
    std::cout << "\n=== Testing Tokenizer Training ===" << std::endl;

    std::string corpus = "hugging face is a company based in New York City";
    std::string prefix = "test_tokenizer";
    int vocab_size = 300;

    // 1. Train a new tokenizer
    Tokenizer tokenizer1(""); // Create an empty tokenizer
    tokenizer1.train(corpus, vocab_size, true);

    // 2. Save the tokenizer
    tokenizer1.save(prefix);

    // 3. Load the tokenizer
    Tokenizer tokenizer2(prefix);

    // 4. Test encoding and decoding
    std::string text_to_encode = "hugging face";
    std::vector<int> encoded = tokenizer2.encode(text_to_encode);
    std::string decoded = tokenizer2.decode(encoded);

    if (decoded == text_to_encode) {
        std::cout << "  Training and Save/Load Passed\n";
    } else {
        std::cout << "  Training and Save/Load FAILED (Expected: \"" << text_to_encode << "\", Got: \"" << decoded << "\")\n";
        throw std::runtime_error("Tokenizer training failed.");
    }
}

int main() {
    try {
        test_tokenizer();
        test_tokenizer_training();
        std::cout << "All Tokenizer tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Tokenizer tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

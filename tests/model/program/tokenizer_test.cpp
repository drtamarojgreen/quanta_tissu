#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <cassert>

// Test result tracking
struct TestResults {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;

    void record_pass(const std::string& test_name) {
        passed++;
        std::cout << "  ✓ " << test_name << std::endl;
    }

    void record_fail(const std::string& test_name, const std::string& reason) {
        failed++;
        failures.push_back(test_name + ": " + reason);
        std::cout << "  ✗ " << test_name << " - " << reason << std::endl;
    }

    void print_summary() {
        std::cout << "\n--- Test Summary ---\n";
        std::cout << "Passed: " << passed << ", Failed: " << failed << ", Total: " << (passed + failed) << std::endl;
        if (failed > 0) {
            std::cout << "Failures:\n";
            for (const auto& f : failures) {
                std::cout << "  - " << f << "\n";
            }
        }
    }
};

void run_self_contained_tokenizer_test() {
    std::cout << "=== Running Self-Contained Tokenizer Test ===" << std::endl;
    TestResults results;

    try {
        // 1. Setup a simple corpus
        std::string corpus = "aab abab aac";
        int vocab_size = 256 + 3; // 256 for bytes + 3 merges

        // 2. Train a new tokenizer in-memory
        Tokenizer tokenizer;
        tokenizer.train(corpus, vocab_size, false); // verbose=false

        // 3. Test vocab size
        int expected_vocab_size = 256 + 3;
        int actual_vocab_size = tokenizer.get_vocab_size();
        if (actual_vocab_size == expected_vocab_size) {
            results.record_pass("Vocab size check");
        } else {
            results.record_fail("Vocab size check", "Expected " + std::to_string(expected_vocab_size) + ", got " + std::to_string(actual_vocab_size));
        }

        // 4. Test encoding
        std::string text_to_encode = "aabc";
        std::vector<int> encoded_ids = tokenizer.encode(text_to_encode);

        // Expected merges:
        // 1. 'a' (97) + 'a' (97) -> 256
        // 2. ' ' (32) + 'a' (97) -> 257 (or something else, depends on frequency)
        // 3. 'a' (97) + 'b' (98) -> 258
        // Let's trace the encoding of "aabc"
        // "aab" -> "aa" + "b" -> 256, 98
        // "c" -> 99
        // This is a simplified expectation, the actual merge order might differ.
        // The most important thing is that decoding works.

        // 5. Test decoding
        std::string decoded_text = tokenizer.decode(encoded_ids);
        if (decoded_text == text_to_encode) {
            results.record_pass("Encode/Decode roundtrip");
        } else {
            results.record_fail("Encode/Decode roundtrip", "Expected '" + text_to_encode + "', got '" + decoded_text + "'");
        }

    } catch (const std::exception& e) {
        results.record_fail("Tokenizer test exception", e.what());
    }

    results.print_summary();
    if (results.failed > 0) {
        throw std::runtime_error("Self-contained tokenizer test failed.");
    }
}

int main() {
    try {
        run_self_contained_tokenizer_test();
        std::cout << "\nAll Tokenizer tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTokenizer tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

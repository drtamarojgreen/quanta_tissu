#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include "test_summary.h"
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <fstream>
#include <sstream>

TestResults results;

// Helper function to read a file into a string
std::string read_file_to_string(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void test_tokenizer() {
    std::cout << "\n=== Testing Tokenizer ===" << std::endl;

    try {
        // The Tokenizer constructor expects a prefix. The test runner executes from the build directory,
        // so we need to point it to the source directory where the tokenizer files are.
        TissLM::Tokenizer::Tokenizer tokenizer(TestConfig::TokenizerPath);

        // Test get_vocab_size
        int vocab_size = tokenizer.get_vocab_size();
        if (vocab_size > 0) {
            results.record_pass("get_vocab_size (Size: " + std::to_string(vocab_size) + ")");
        } else {
            results.record_fail("get_vocab_size", "Expected a positive size, Got " + std::to_string(vocab_size));
        }

        // Test encode
        std::string text_to_encode = "ab cd";
        std::vector<int> encoded_ids = tokenizer.encode(text_to_encode);
        std::vector<int> expected_ids = {4, 32, 5}; // 'ab' -> 4, ' ' -> 32, 'cd' -> 5

        if (encoded_ids == expected_ids) {
            results.record_pass("Encode");
        } else {
            results.record_fail("Encode", "ID mismatch");
        }

        // Test decode
        std::string decoded_text = tokenizer.decode(encoded_ids);
        std::string expected_text = "ab cd"; // Assuming space is handled correctly

        if (decoded_text == expected_text) {
            results.record_pass("Decode");
        } else {
            results.record_fail("Decode", "Expected: \"" + expected_text + "\", Got: \"" + decoded_text + "\"");
        }
    } catch (const std::exception& e) {
        results.record_fail("Tokenizer basic tests", e.what());
    }
}

void test_tokenizer_training() {
    std::cout << "\n=== Testing Tokenizer Training ===" << std::endl;

    try {
        std::string corpus = "Cognitive Behavioral Therapy helps individuals identify and change destructive thinking patterns.";
        std::string prefix = "test_cbt_tokenizer";
        int vocab_size = 300;

        // 1. Train a new tokenizer
        TissLM::Tokenizer::Tokenizer tokenizer1(""); // Create an empty tokenizer
        tokenizer1.train(corpus, vocab_size, false);

        // 2. Save the tokenizer
        tokenizer1.save(prefix);

        // 3. Load the tokenizer
        TissLM::Tokenizer::Tokenizer tokenizer2(prefix);

        // 4. Test encoding and decoding
        std::string text_to_encode = "change thinking patterns";
        std::vector<int> encoded = tokenizer2.encode(text_to_encode);
        std::string decoded = tokenizer2.decode(encoded);

        if (decoded == text_to_encode) {
            results.record_pass("Training and Save/Load");
        } else {
            results.record_fail("Training and Save/Load", "Expected: \"" + text_to_encode + "\", Got: \"" + decoded + "\"");
        }
    } catch (const std::exception& e) {
        results.record_fail("Tokenizer training", e.what());
    }
}

void test_corpus_based_tokenizer() {
    std::cout << "\n=== Testing Corpus-Based Tokenizer Training ===" << std::endl;

    try {
        // 1. Read the corpus
        std::string corpus_path = "../../../test_corpus/cbt_corpus.txt";
        std::string corpus = read_file_to_string(corpus_path);
        if (corpus.empty()) {
            results.record_fail("Corpus load", "Corpus file is empty or could not be read");
            return;
        }
        results.record_pass("Corpus loaded successfully");

        // 2. Train a new tokenizer on the corpus
        std::string prefix = "corpus_trained_tokenizer";
        int vocab_size = 400; // A bit larger for a real corpus
        TissLM::Tokenizer::Tokenizer tokenizer("");
        tokenizer.train(corpus, vocab_size, false); // verbose = false

        // 3. Save and reload the tokenizer to ensure serialization works
        tokenizer.save(prefix);
        TissLM::Tokenizer::Tokenizer reloaded_tokenizer(prefix);

        // 4. Test encoding and decoding
        std::string text_to_test = "solving current problems";
        std::vector<int> encoded = reloaded_tokenizer.encode(text_to_test);
        std::string decoded = reloaded_tokenizer.decode(encoded);

        if (decoded == text_to_test) {
            results.record_pass("Corpus-based Training and Save/Load");
        } else {
            results.record_fail("Corpus-based Training and Save/Load", "Expected: \"" + text_to_test + "\", Got: \"" + decoded + "\"");
        }
    } catch (const std::exception& e) {
        results.record_fail("Corpus-based tokenizer test", e.what());
    }
}

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "TissLM C++ Tokenizer Test Suite" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    try {
        test_tokenizer();
        test_tokenizer_training();
        test_corpus_based_tokenizer();

        results.print_summary();
        return (results.failed == 0) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Tokenizer tests failed with unhandled exception: " << e.what() << std::endl;
        return 1;
    }
}

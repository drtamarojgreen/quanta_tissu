#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <fstream>
#include <sstream>

using namespace TissLM::Tokenizer;

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
    std::cout << "=== Testing Tokenizer ===" << std::endl;

    // The Tokenizer constructor expects a prefix. The test runner executes from the build directory,
    // so we need to point it to the source directory where the tokenizer files are.
    Tokenizer tokenizer(TestConfig::TokenizerPath);

    // Test get_vocab_size
    int vocab_size = tokenizer.get_vocab_size();
    if (vocab_size > 0) {
        std::cout << "  get_vocab_size Passed (Size: " << vocab_size << ")\n";
    } else {
        std::cout << "  get_vocab_size FAILED (Expected a positive size, Got " << vocab_size << ")\n";
        throw std::runtime_error("Tokenizer get_vocab_size failed.");
    }

    // Test encode
    std::string text_to_encode = "ab cd";
    std::vector<int> encoded_ids = tokenizer.encode(text_to_encode);
    std::vector<int> expected_ids = {4, 32, 5}; // 'ab' -> 4, ' ' -> 32, 'cd' -> 5

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
        std::cout << "  Decode FAILED (Expected: \"" << expected_text << "\", Got: \"" << decoded_text << "\")\n";
        throw std::runtime_error("Tokenizer decode failed.");
    }

    std::cout << "Tokenizer tests completed successfully." << std::endl << std::endl;
}

void test_tokenizer_training() {
    std::cout << "\n=== Testing Tokenizer Training ===" << std::endl;

    std::string corpus = "Cognitive Behavioral Therapy helps individuals identify and change destructive thinking patterns.";
    std::string prefix = "test_cbt_tokenizer";
    int vocab_size = 300;

    // 1. Train a new tokenizer
    Tokenizer tokenizer1(""); // Create an empty tokenizer
    tokenizer1.train(corpus, vocab_size, true);

    // 2. Save the tokenizer
    tokenizer1.save(prefix);

    // 3. Load the tokenizer
    Tokenizer tokenizer2(prefix);

    // 4. Test encoding and decoding
    std::string text_to_encode = "change thinking patterns";
    std::vector<int> encoded = tokenizer2.encode(text_to_encode);
    std::string decoded = tokenizer2.decode(encoded);

    if (decoded == text_to_encode) {
        std::cout << "  Training and Save/Load Passed\n";
    } else {
        std::cout << "  Training and Save/Load FAILED (Expected: \"" << text_to_encode << "\", Got: \"" << decoded << "\")\n";
        throw std::runtime_error("Tokenizer training failed.");
    }
}

void test_corpus_based_tokenizer() {
    std::cout << "\n=== Testing Corpus-Based Tokenizer Training ===" << std::endl;

    // 1. Read the corpus
    std::string corpus_path = "../../../test_corpus/cbt_corpus.txt";
    std::string corpus = read_file_to_string(corpus_path);
    if (corpus.empty()) {
        throw std::runtime_error("Corpus file is empty or could not be read.");
    }
    std::cout << "  Corpus loaded successfully from: " << corpus_path << std::endl;

    // 2. Train a new tokenizer on the corpus
    std::string prefix = "corpus_trained_tokenizer";
    int vocab_size = 400; // A bit larger for a real corpus
    Tokenizer tokenizer("");
    tokenizer.train(corpus, vocab_size, false); // verbose = false

    // 3. Save and reload the tokenizer to ensure serialization works
    tokenizer.save(prefix);
    Tokenizer reloaded_tokenizer(prefix);

    // 4. Test encoding and decoding
    std::string text_to_test = "solving current problems";
    std::vector<int> encoded = reloaded_tokenizer.encode(text_to_test);
    std::string decoded = reloaded_tokenizer.decode(encoded);

    if (decoded == text_to_test) {
        std::cout << "  Corpus-based Training and Save/Load Passed\n";
    } else {
        std::cout << "  Corpus-based Training and Save/Load FAILED (Expected: \"" << text_to_test << "\", Got: \"" << decoded << "\")\n";
        throw std::runtime_error("Corpus-based tokenizer test failed.");
    }
}

int main() {
    try {
        test_tokenizer();
        test_tokenizer_training();
        test_corpus_based_tokenizer();
        std::cout << "\nAll Tokenizer tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Tokenizer tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

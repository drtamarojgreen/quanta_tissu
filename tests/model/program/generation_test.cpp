#include "generation/generator.h"
#include "generation/generation_config.h"
#include "tokenizer/tokenizer.h"
#include "core/transformer_model.h"
#include <memory>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

using namespace TissDB::TissLM::Core;
using namespace TissDB::TissLM::Generation;
using namespace TissDB::TissLM::Tokenizer;

void run_generation_test() {
    // 1. Setup Tokenizer
    std::cout << "Initializing tokenizer..." << std::endl;
    Tokenizer tokenizer("models/tokenizers/revised_tokenizer");
    int vocab_size = tokenizer.get_vocab_size();
    assert(vocab_size > 0 && "Tokenizer must have a vocabulary size greater than 0.");
    std::cout << "Tokenizer initialized with vocab size: " << vocab_size << std::endl;

    // 2. Setup Model
    std::cout << "Initializing model..." << std::endl;
    int n_layer = 2;
    int n_head = 2;
    int n_embd = 64;
    int block_size = 128;
    float dropout = 0.1f;
    auto model = std::make_shared<TransformerModel>(vocab_size, n_layer, n_head, n_embd, block_size, dropout);
    std::cout << "Model initialized." << std::endl;

    // 3. Setup Generator
    std::cout << "Initializing generator..." << std::endl;
    GenerationConfig config = GenerationConfig::greedy();
    Generator generator(model, config);
    std::cout << "Generator initialized." << std::endl;

    // 4. Generate Text
    std::string prompt_text = "Hello, world!";
    std::cout << "Encoding prompt: \"" << prompt_text << "\"" << std::endl;
    std::vector<int> prompt_tokens = tokenizer.encode(prompt_text);

    int max_new_tokens = 10;
    std::cout << "Generating " << max_new_tokens << " new tokens..." << std::endl;
    std::vector<int> generated_tokens = generator.generate(prompt_tokens, max_new_tokens);

    // 5. Verify Output
    std::cout << "Verifying output..." << std::endl;
    assert(generated_tokens.size() == prompt_tokens.size() + max_new_tokens && "Generated sequence has incorrect length.");

    std::string decoded_text = tokenizer.decode(generated_tokens);
    std::cout << "Generated Text: " << decoded_text << std::endl;

    assert(decoded_text.rfind(prompt_text, 0) == 0 && "Generated text does not start with the prompt.");
    assert(decoded_text.length() > prompt_text.length() && "Generated text is not longer than the prompt.");

    std::cout << "Generation test passed successfully!" << std::endl;
}

int main() {
    try {
        run_generation_test();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with an unknown exception." << std::endl;
        return 1;
    }
    return 0;
}
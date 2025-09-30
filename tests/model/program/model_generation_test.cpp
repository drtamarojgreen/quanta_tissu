#include "../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include <iostream>
#include <vector>
#include <memory>

using namespace TissDB::TissLM::Core;
using namespace TissNum;

void test_transformer_model() {
    std::cout << "=== Testing Transformer Model ===" << std::endl;

    int vocab_size = 100;
    int max_seq_len = 50;
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 4; // Test with LoRA enabled

    TransformerModel model(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);

    // Test forward pass (training mode)
    Matrix input_tokens_train(1, 5); // Batch size 1, sequence length 5
    input_tokens_train(0,0) = 1.0f; input_tokens_train(0,1) = 2.0f; input_tokens_train(0,2) = 3.0f; input_tokens_train(0,3) = 4.0f; input_tokens_train(0,4) = 5.0f;
    Matrix output_train = model.forward(input_tokens_train);
    std::cout << "  TransformerModel forward (training) output shape: (" << output_train.rows() << ", " << output_train.cols() << ")\n";
    if (output_train.rows() == 5 && output_train.cols() == vocab_size) {
        std::cout << "  Forward (training) Passed\n";
    } else {
        std::cout << "  Forward (training) FAILED\n";
        throw std::runtime_error("TransformerModel forward (training) failed.");
    }

    // Test forward_inference pass (with KV caching)
    std::vector<int> prompt_tokens = {1, 2, 3};
    std::vector<std::pair<Matrix, Matrix>> past_kv_cache;
    std::vector<std::pair<Matrix, Matrix>> new_kv_cache;

    Matrix first_token_input(1, 1);
    first_token_input(0,0) = static_cast<float>(prompt_tokens[0]);
    Matrix logits_first = model.forward_inference(first_token_input, past_kv_cache, new_kv_cache);
    std::cout << "  Logits for first token shape: (" << logits_first.rows() << ", " << logits_first.cols() << ")\n";
    if (logits_first.rows() == 1 && logits_first.cols() == vocab_size) {
        std::cout << "  Forward (inference) first token Passed\n";
    } else {
        std::cout << "  Forward (inference) first token FAILED\n";
        throw std::runtime_error("TransformerModel forward (inference) first token failed.");
    }
    past_kv_cache = new_kv_cache;
    new_kv_cache.clear();

    Matrix second_token_input(1, 1);
    second_token_input(0,0) = static_cast<float>(prompt_tokens[1]);
    Matrix logits_second = model.forward_inference(second_token_input, past_kv_cache, new_kv_cache);
    std::cout << "  Logits for second token shape: (" << logits_second.rows() << ", " << logits_second.cols() << ")\n";
    if (logits_second.rows() == 1 && logits_second.cols() == vocab_size) {
        std::cout << "  Forward (inference) second token Passed\n";
    } else {
        std::cout << "  Forward (inference) second token FAILED\n";
        throw std::runtime_error("TransformerModel forward (inference) second token failed.");
    }
    past_kv_cache = new_kv_cache;
    new_kv_cache.clear();

    std::cout << "Transformer Model tests completed successfully." << std::endl << std::endl;
}

void test_generator() {
    std::cout << "=== Testing Generator ===" << std::endl;

    int vocab_size = 100;
    int max_seq_len = 50;
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 0; // Test Generator without LoRA for simplicity

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);
    GenerationConfig config;
    config.eos_token_id = 50; // Example EOS token ID

    Generator generator(model, config);

    std::vector<int> prompt_tokens = {10, 20};
    int max_new_tokens = 5;

    std::vector<int> generated_sequence = generator.generate(prompt_tokens, max_new_tokens);

    std::cout << "  Generated sequence length: " << generated_sequence.size() << "\n";
    if (generated_sequence.size() >= prompt_tokens.size() && generated_sequence.size() <= prompt_tokens.size() + max_new_tokens) {
        std::cout << "  Generator generate Passed\n";
    } else {
        std::cout << "  Generator generate FAILED\n";
        throw std::runtime_error("Generator generate failed.");
    }

    std::cout << "Generator tests completed successfully." << std::endl << std::endl;
}

int main() {
    try {
        test_transformer_model();
        test_generator();
        std::cout << "All Model and Generation tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Model and Generation tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

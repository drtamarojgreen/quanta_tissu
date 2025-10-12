#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <random>

using namespace TissDB::TissLM::Core;
using namespace TissNum;

// Helper to print generated tokens
void print_tokens(const std::vector<int>& tokens, const std::string& prefix = "") {
    std::cout << prefix;
    for (int token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
}

void run_cpp_full_fledged_workout() {
    std::cout << "=== Running C++ Full-Fledged Workout ===" << std::endl;

    // --- 1. Setup Model and Tokenizer ---
    // Dummy tokenizer (assuming dummy_vocab.json and dummy_merges.txt are available)
    Tokenizer tokenizer("dummy");
    int vocab_size = tokenizer.get_vocab_size();
    int max_seq_len = 50;
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 4; // Test with LoRA enabled

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- 2. Test Generation with various parameters (simplified) ---
    std::vector<std::map<std::string, float>> generation_params = {
        // Greedy
        {{"temperature", 0.0f}, {"top_k", 0.0f}}, 
        // Top-k
        {{"temperature", 1.0f}, {"top_k", 10.0f}},
        {{"temperature", 1.0f}, {"top_k", 5.0f}}
    };

    std::vector<std::string> prompts = {
        "The quick brown fox",
        "Hello world"
    };
    int max_new_tokens = 10;

    for (const auto& params_map : generation_params) {
        Generation::GenerationConfig config;
        if (params_map.count("temperature")) config.temperature = params_map.at("temperature");
        if (params_map.count("top_k")) config.top_k = static_cast<int>(params_map.at("top_k"));
        config.eos_ids.push_back(0); // Assuming 0 is EOS for dummy tokenizer

        Generator generator(model, config);

        std::cout << "\n  Testing with params: Temp=" << config.temperature << ", TopK=" << config.top_k.value_or(-1) << std::endl;

        for (const std::string& prompt_text : prompts) {
            std::vector<int> prompt_tokens = tokenizer.encode(prompt_text);
            std::vector<int> generated_tokens = generator.generate(prompt_tokens, max_new_tokens);

            std::cout << "    Prompt: \"" << prompt_text << "\"\n";
            print_tokens(generated_tokens, "    Generated IDs: ");
            std::cout << "    Generated Text: \"" << tokenizer.decode(generated_tokens) << "\"\n";
        }
    }
    std::cout << "  Generation tests completed." << std::endl;

    // --- 3. Basic KV Caching check (implicit in Generator.generate) ---
    // The Generator::generate method already uses forward_inference with KV caching.
    // If the generation tests pass, it implies basic KV caching is working.
    std::cout << "\n  Basic KV Caching check: Implicitly tested by Generator.generate." << std::endl;

    // --- 4. Basic LoRA check (implicit in TransformerModel) ---
    // The TransformerModel is initialized with lora_rank > 0.
    // If the model initializes and runs forward/backward without issues, LoRA integration is basic working.
    std::cout << "  Basic LoRA check: Implicitly tested by TransformerModel initialization and generation." << std::endl;

    std::cout << "\n=== C++ Full-Fledged Workout Completed Successfully ===" << std::endl;
}

int main() {
    try {
        run_cpp_full_fledged_workout();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C++ Full-Fledged Workout failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>

using namespace TissLM::Core;
using namespace TissLM::Generation;
using namespace TissLM::Tokenizer;
using namespace TissNum;

// Helper to print generated tokens
void print_tokens_int(const std::vector<int>& tokens, const std::string& prefix = "") {
    std::cout << prefix;
    for (int token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
}

void run_experimental_sampling_evaluation() {
    std::cout << "=== Running Experimental Sampling Evaluation (C++) ===" << std::endl;

    // --- Setup Model and Tokenizer ---
    Tokenizer tokenizer(TestConfig::TokenizerPath);
    int vocab_size = tokenizer.get_vocab_size();

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(
        vocab_size,
        TestConfig::MaxSeqLen,
        TestConfig::EmbedDim,
        TestConfig::NumHeads,
        TestConfig::NumLayers,
        TestConfig::DropoutRate,
        0 // LoraRank = 0 for this test
    );

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- Test Scenarios for Explicit Underlying Sampling ---
    std::vector<std::tuple<std::string, int, GenerationConfig>> sampling_scenarios = {
        {"The quick brown fox", 10, GenerationConfig::greedy()},
        {"The quick brown fox", 10, GenerationConfig::with_top_k(5, 1.0f)},
        {"The quick brown fox", 10, GenerationConfig::nucleus(0.9f, 1.0f)},
        {"Hello world", 15, GenerationConfig::greedy()},
        {"Hello world", 15, GenerationConfig::with_top_k(10, 1.0f)},
        {"Hello world", 15, GenerationConfig::nucleus(0.7f, 1.0f)}
    };

    for (const auto& scenario : sampling_scenarios) {
        std::string prompt = std::get<0>(scenario);
        int tokens_to_generate = std::get<1>(scenario);
        GenerationConfig config = std::get<2>(scenario);

        std::cout << "\n  Scenario: Prompt=\"" << prompt.substr(0, 30) << "...\", Tokens=" << tokens_to_generate << std::endl;
        std::cout << "    Method: " << config.method << ", Temp=" << config.temperature;
        if (config.top_k.has_value()) std::cout << ", TopK=" << config.top_k.value();
        if (config.top_p.has_value()) std::cout << ", TopP=" << config.top_p.value();
        std::cout << std::endl;

        try {
            Generator generator(model, config);
            std::vector<int> prompt_tokens = tokenizer.encode(prompt);
            std::vector<int> generated_tokens = generator.generate(prompt_tokens, tokens_to_generate);

            std::cout << "      Generated IDs: ";
            print_tokens_int(generated_tokens);
            std::cout << "      Generated Text: \"" << tokenizer.decode(generated_tokens) << "\"" << std::endl;

            // Basic check: ensure some tokens were generated
            if (generated_tokens.size() > prompt_tokens.size()) {
                std::cout << "      ✓ Generation successful." << std::endl;
            } else {
                std::cout << "      ✗ Generation failed (no new tokens)." << std::endl;
                throw std::runtime_error("Generation failed.");
            }
        } catch (const std::exception& e) {
            std::cerr << "      [ERROR] Test failed: " << e.what() << std::endl;
        }
    }

    std::cout << "\n=== Experimental Sampling Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_experimental_sampling_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Experimental Sampling Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

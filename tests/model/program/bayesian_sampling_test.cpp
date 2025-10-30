#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>

using namespace TissLM::Core;
using namespace TissLM::Generation;
using namespace TissLM::Tokenizer;

// Helper to print a vector
void print_vector(const std::vector<int>& vec) {
    for (int val : vec) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    try {
        std::cout << "=== Testing Bayesian Sampling ===\n" << std::endl;

        // 1. Create a model and tokenizer
        Tokenizer tokenizer(TestConfig::TokenizerPath);
        int vocab_size = tokenizer.get_vocab_size();
        auto model = std::make_shared<TransformerModel>(
            vocab_size,
            TestConfig::MaxSeqLen,
            TestConfig::EmbedDim,
            TestConfig::NumHeads,
            TestConfig::NumLayers,
            TestConfig::FFNDim,
            TestConfig::DropoutRate
        );

        // 2. Set up a generator with a bayesian_influenced config
        GenerationConfig config;
        config.method = "bayesian_influenced";
        config.query_embedding = std::vector<float>(TestConfig::EmbedDim, 0.5f);
        config.eigenvalues = std::vector<float>(TestConfig::EmbedDim, 0.1f);
        config.bayesian_influence_scale = 0.5f;

        std::vector<int> prompt = {1, 2, 3};
        int max_new_tokens = 10;

        // --- Test 1: Non-determinism ---
        std::cout << "--- Test 1: Non-determinism ---" << std::endl;
        Generator generator1(model, config);
        std::vector<int> generated_tokens1 = generator1.generate(prompt, max_new_tokens);

        Generator generator2(model, config);
        std::vector<int> generated_tokens2 = generator2.generate(prompt, max_new_tokens);

        if (generated_tokens1 != generated_tokens2) {
            std::cout << "  [PASSED] Generated sequences are different, indicating non-determinism." << std::endl;
        } else {
            std::cout << "  [FAILED] Generated sequences are identical, indicating a lack of non-determinism." << std::endl;
            std::cout << "    Run 1: "; print_vector(generated_tokens1);
            std::cout << "    Run 2: "; print_vector(generated_tokens2);
            return 1;
        }

        // --- Test 2: Reproducibility with seed ---
        std::cout << "\n--- Test 2: Reproducibility with seed ---" << std::endl;
        config.seed = 42;
        Generator seeded_generator1(model, config);
        std::vector<int> seeded_tokens1 = seeded_generator1.generate(prompt, max_new_tokens);

        Generator seeded_generator2(model, config);
        std::vector<int> seeded_tokens2 = seeded_generator2.generate(prompt, max_new_tokens);

        if (seeded_tokens1 == seeded_tokens2) {
            std::cout << "  [PASSED] Generated sequences with the same seed are identical." << std::endl;
        } else {
            std::cout << "  [FAILED] Generated sequences with the same seed are different." << std::endl;
            std::cout << "    Run 1: "; print_vector(seeded_tokens1);
            std::cout << "    Run 2: "; print_vector(seeded_tokens2);
            return 1;
        }

        // --- Test 3: Output properties ---
        std::cout << "\n--- Test 3: Output properties ---" << std::endl;
        // Check length
        size_t expected_size = prompt.size() + max_new_tokens;
        if (seeded_tokens1.size() == expected_size) {
            std::cout << "  [PASSED] Generated sequence has the correct length (" << expected_size << ")." << std::endl;
        } else {
            std::cout << "  [FAILED] Generated sequence has incorrect length. Expected: " << expected_size << ", Got: " << seeded_tokens1.size() << std::endl;
            return 1;
        }

        // Check vocabulary constraints
        bool all_in_vocab = true;
        for (int token_id : seeded_tokens1) {
            if (token_id < 0 || token_id >= vocab_size) {
                all_in_vocab = false;
                std::cout << "  [FAILED] Generated token ID " << token_id << " is out of vocabulary range [0, " << vocab_size - 1 << "] ." << std::endl;
                break;
            }
        }
        if (all_in_vocab) {
            std::cout << "  [PASSED] All generated tokens are within the vocabulary size." << std::endl;
        } else {
            return 1;
        }

        std::cout << "\nBayesian sampling test completed successfully." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Bayesian sampling test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
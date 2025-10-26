#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>

using namespace TissLM::Core;
using namespace TissLM::Generation;
using namespace TissLM::Tokenizer;

int main() {
    try {
        std::cout << "=== Testing Bayesian Sampling ===" << std::endl;

        // 1. Create a model
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

        Generator generator(model, config);

        // 3. Generate text
        std::vector<int> prompt = {1, 2, 3};
        int max_new_tokens = 10;
        std::vector<int> generated_tokens = generator.generate(prompt, max_new_tokens);

        // 4. Assert something about the output
        if (generated_tokens.size() > prompt.size()) {
            std::cout << "  [PASSED] Bayesian sampling generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "  [FAILED] Bayesian sampling did not generate new tokens." << std::endl;
            return 1;
        }

        std::cout << "Bayesian sampling test completed successfully." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Bayesian sampling test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

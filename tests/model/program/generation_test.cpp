#include "gtest/gtest.h"
#include "quanta_tissu/tisslm/program/generation/generator.h"
#include "quanta_tissu/tisslm/program/generation/generation_config.h"
#include "quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include <memory>
#include <iostream>

using namespace TissDB::TissLM::Core;
using namespace TissDB::TissLM::Generation;
using namespace TissDB::TissLM::Tokenizer;

TEST(GenerationTest, BasicGeneration) {
    // 1. Setup Tokenizer
    // Note: User has specified that tokenizer files are assumed to exist.
    Tokenizer tokenizer("models/tokenizers/revised_tokenizer");
    int vocab_size = tokenizer.get_vocab_size();
    ASSERT_GT(vocab_size, 0);

    // 2. Setup Model
    int n_layer = 2;
    int n_head = 2;
    int n_embd = 64;
    int block_size = 128;
    float dropout = 0.1f;
    auto model = std::make_shared<TransformerModel>(vocab_size, n_layer, n_head, n_embd, block_size, dropout);

    // 3. Setup Generator
    GenerationConfig config = GenerationConfig::greedy();
    Generator generator(model, config);

    // 4. Generate Text
    std::string prompt_text = "Hello, world!";
    std::vector<int> prompt_tokens = tokenizer.encode(prompt_text);

    int max_new_tokens = 10;
    std::vector<int> generated_tokens = generator.generate(prompt_tokens, max_new_tokens);

    // 5. Verify Output
    EXPECT_EQ(generated_tokens.size(), prompt_tokens.size() + max_new_tokens);

    std::string decoded_text = tokenizer.decode(generated_tokens);
    std::cout << "Generated Text: " << decoded_text << std::endl;

    EXPECT_TRUE(decoded_text.rfind(prompt_text, 0) == 0); // Check if it starts with the prompt
    EXPECT_GT(decoded_text.length(), prompt_text.length());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
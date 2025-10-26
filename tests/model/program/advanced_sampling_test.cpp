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

void run_advanced_sampling_evaluation() {
    std::cout << "=== Running Advanced Sampling Evaluation (C++) ===" << std::endl;

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

    // --- Test 10a: Beam Search ---
    std::cout << "\n  --- Test 10a: Beam Search ---" << std::endl;
    try {
        std::string prompt = "A helpful way to manage anxiety is to";
        int length = 20;
        int beam_width = 3;

        GenerationConfig config = GenerationConfig::greedy();
        config.method = "beam_search";
        config.beam_width = beam_width;

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.beam_search(prompt_tokens, length, beam_width, -1);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;
        if (generated_tokens.size() > prompt_tokens.size()) {
            std::cout << "    [PASSED] Beam Search generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "    [FAILED] Beam Search did not generate new tokens." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Beam Search test failed: " << e.what() << std::endl;
    }

    // --- Test 10b: Contrastive Search ---
    std::cout << "\n  --- Test 10b: Contrastive Search ---" << std::endl;
    try {
        std::string prompt = "The core belief behind impostor syndrome is";
        int length = 30;
        int beam_width = 5;
        float alpha = 0.6;

        GenerationConfig config;
        config.method = "contrastive_search";
        config.beam_width = beam_width;
        config.contrastive_alpha = alpha;

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.contrastive_search(prompt_tokens, length, beam_width, alpha, -1);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;
        if (generated_tokens.size() > 0) {
            std::cout << "    [PASSED] Contrastive Search generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "    [FAILED] Contrastive Search did not generate new tokens." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Contrastive Search test failed: " << e.what() << std::endl;
    }

    // --- Test 10c: Mirostat Sampling ---
    std::cout << "\n  --- Test 10c: Mirostat Sampling ---" << std::endl;
    try {
        std::string prompt = "In a session focused on exposure therapy,";
        int length = 50;
        float tau = 5.0;
        float eta = 0.1;

        GenerationConfig config;
        config.method = "mirostat_sampling";
        config.mirostat_tau = tau;
        config.mirostat_eta = eta;

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.mirostat_sampling(prompt_tokens, length, tau, eta, -1);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;
        if (generated_tokens.size() > 0) {
            std::cout << "    [PASSED] Mirostat Sampling generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "    [FAILED] Mirostat Sampling did not generate new tokens." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Mirostat Sampling test failed: " << e.what() << std::endl;
    }

    // --- Test 10d: N-gram Repetition Penalty ---
    std::cout << "\n  --- Test 10d: N-gram Repetition Penalty ---" << std::endl;
    try {
        std::string prompt = "The quick brown fox jumps over the lazy dog. The quick brown fox";
        int length = 10;
        int no_repeat_ngram_size = 4;

        GenerationConfig config;
        config.no_repeat_ngram_size = no_repeat_ngram_size;

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.generate(prompt_tokens, length);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;

        // Check if the generated part contains the repeated n-gram
        std::string generated_part = decoded_text.substr(prompt.length());
        if (generated_part.find("jumps") == std::string::npos) {
            std::cout << "    [PASSED] N-gram repetition penalty seems to be working." << std::endl;
        } else {
            std::cout << "    [FAILED] N-gram repetition penalty might not be working." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] N-gram Repetition Penalty test failed: " << e.what() << std::endl;
    }

    // --- Test 10e: Constrained Text Generation (Logit Bias) ---
    std::cout << "\n  --- Test 10e: Constrained Text Generation (Logit Bias) ---" << std::endl;
    try {
        std::string prompt = "The next word is";
        int length = 1;

        // Force the next token to be "apple"
        int apple_token_id = tokenizer.encode(" apple")[0];

        GenerationConfig config;
        config.logit_bias[apple_token_id] = 100.0; // Add a large bias to the token for "apple"

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.generate(prompt_tokens, length);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;

        if (decoded_text.find("apple") != std::string::npos) {
            std::cout << "    [PASSED] Logit bias seems to be working." << std::endl;
        } else {
            std::cout << "    [FAILED] Logit bias might not be working." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Constrained Text Generation test failed: " << e.what() << std::endl;
    }

    // --- Test 10f: Temperature Scheduling ---
    std::cout << "\n  --- Test 10f: Temperature Scheduling ---" << std::endl;
    try {
        std::string prompt = "The temperature is";
        int length = 10;

        GenerationConfig config;
        config.temperature_schedule = {1.0, 0.5, 0.1};

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.generate(prompt_tokens, length);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;
        if (generated_tokens.size() > prompt.size()) {
            std::cout << "    [PASSED] Temperature Scheduling generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "    [FAILED] Temperature Scheduling did not generate new tokens." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Temperature Scheduling test failed: " << e.what() << std::endl;
    }

    // --- Test 10g: Top-A Sampling ---
    std::cout << "\n  --- Test 10g: Top-A Sampling ---" << std::endl;
    try {
        std::string prompt = "The quick brown fox";
        int length = 10;
        float top_a = 0.1;

        GenerationConfig config;
        config.method = "top_a";
        config.top_a = top_a;

        Generator generator(model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.generate(prompt_tokens, length);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;
        if (generated_tokens.size() > prompt.size()) {
            std::cout << "    [PASSED] Top-A Sampling generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "    [FAILED] Top-A Sampling did not generate new tokens." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Top-A Sampling test failed: " << e.what() << std::endl;
    }

    // --- Test 10h: EOS Token Handling ---
    std::cout << "\n  --- Test 10h: EOS Token Handling ---" << std::endl;
    try {
        std::string prompt = "This is a test";
        int length = 10;
        int eos_token_id = tokenizer.encode(" test")[0];

        // Test 1: suppress_eos = false
        {
            GenerationConfig config;
            config.eos_ids = {eos_token_id};
            config.suppress_eos = false;
            config.logit_bias[eos_token_id] = 100.0; // Force EOS generation

            Generator generator(model, config);
            std::vector<int> prompt_tokens = tokenizer.encode(prompt);
            std::vector<int> generated_tokens = generator.generate(prompt_tokens, length);

            if (!generated_tokens.empty() && generated_tokens.back() == eos_token_id) {
                std::cout << "    [PASSED] EOS token was correctly included." << std::endl;
            } else {
                std::cout << "    [FAILED] EOS token was not included when suppress_eos is false." << std::endl;
            }
        }

        // Test 2: suppress_eos = true
        {
            GenerationConfig config;
            config.eos_ids = {eos_token_id};
            config.suppress_eos = true;
            config.logit_bias[eos_token_id] = 100.0; // Force EOS generation

            Generator generator(model, config);
            std::vector<int> prompt_tokens = tokenizer.encode(prompt);
            std::vector<int> generated_tokens = generator.generate(prompt_tokens, length);

            if (generated_tokens.empty() || generated_tokens.back() != eos_token_id) {
                std::cout << "    [PASSED] EOS token was correctly suppressed." << std::endl;
            } else {
                std::cout << "    [FAILED] EOS token was not suppressed when suppress_eos is true." << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] EOS Token Handling test failed: " << e.what() << std::endl;
    }

    // --- Test 10i: Batched Text Generation ---
    std::cout << "\n  --- Test 10i: Batched Text Generation ---" << std::endl;
    try {
        std::vector<std::string> prompts = {
            "The first prompt.",
            "The second prompt."
        };
        int length = 5;

        GenerationConfig config;
        Generator generator(model, config);

        std::vector<std::vector<int>> prompt_tokens;
        for (const auto& p : prompts) {
            prompt_tokens.push_back(tokenizer.encode(p));
        }

        std::vector<std::vector<int>> generated_tokens = generator.generate_batch(prompt_tokens, length);

        if (generated_tokens.size() == prompts.size()) {
            std::cout << "    [PASSED] Batched generation returned the correct number of sequences." << std::endl;
        } else {
            std::cout << "    [FAILED] Batched generation returned an incorrect number of sequences." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Batched Text Generation test failed: " << e.what() << std::endl;
    }

    // --- Test 10j: Speculative Sampling ---
    std::cout << "\n  --- Test 10j: Speculative Sampling ---" << std::endl;
    try {
        std::string prompt = "The journey to mental wellness begins with";
        int length = 20;

        // Create a smaller draft model
        auto draft_model = std::make_shared<TransformerModel>(
            vocab_size,
            TestConfig::MaxSeqLen,
            16, // smaller embed_dim
            2,  // smaller num_heads
            1,  // smaller num_layers
            64, // smaller d_ff
            TestConfig::DropoutRate
        );

        GenerationConfig config;
        config.method = "speculative_sampling";

        Generator generator(model, draft_model, config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.speculative_sampling(prompt_tokens, length);

        std::string decoded_text = tokenizer.decode(generated_tokens);
        std::cout << "    Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "    Generated Text: \"" << decoded_text << "\"" << std::endl;
        if (generated_tokens.size() > prompt.size()) {
            std::cout << "    [PASSED] Speculative Sampling generated a sequence of tokens." << std::endl;
        } else {
            std::cout << "    [FAILED] Speculative Sampling did not generate new tokens." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "    [ERROR] Speculative Sampling test failed: " << e.what() << std::endl;
    }

    std::cout << "\n=== Advanced Sampling Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_advanced_sampling_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Advanced Sampling Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

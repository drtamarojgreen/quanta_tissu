#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "../../../quanta_tissu/tisslm/program/training/trainer.h"
#include "../../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../../quanta_tissu/tisslm/program/training/loss_function.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

using namespace TissLM::Core;
using namespace TissLM::Generation;
using namespace TissLM::Tokenizer;
using namespace TissLM::Training;
namespace fs = std::filesystem;

void run_generation_config_tests(std::shared_ptr<TransformerModel> model, Tokenizer& tokenizer) {
    std::cout << "\n=== Running Generation Configuration Tests (Final Model) ===" << std::endl;
    std::string prompt_text = "The quick brown fox";
    std::vector<int> prompt_tokens = tokenizer.encode(prompt_text);
    int gen_len = 50;

    struct ConfigTest {
        std::string name;
        GenerationConfig config;
    };

    std::vector<ConfigTest> tests;

    // 1. Greedy (Baseline)
    tests.push_back({"Greedy", GenerationConfig::greedy()});

    // 2. Temperature Sampling
    // Temp = 0.7
    {
        auto cfg = GenerationConfig::sampling(0.7f);
        tests.push_back({"Temperature (0.7)", cfg});
    }
    // Temp = 1.2
    {
        auto cfg = GenerationConfig::sampling(1.2f);
        tests.push_back({"Temperature (1.2)", cfg});
    }

    // 3. Top-K Sampling
    {
        auto cfg = GenerationConfig::with_top_k(40, 1.0f);
        tests.push_back({"Top-K (K=40)", cfg});
    }

    // 4. Nucleus (Top-P) Sampling
    {
        auto cfg = GenerationConfig::nucleus(0.9f, 1.0f);
        tests.push_back({"Nucleus (P=0.9)", cfg});
    }

    // 5. Repetition Penalty
    {
        auto cfg = GenerationConfig::greedy();
        cfg.repetition_penalty = 1.2f;
        tests.push_back({"Repetition Penalty (1.2)", cfg});
    }

    // 6. Combined: Nucleus + Temp + Repetition Penalty
    {
        auto cfg = GenerationConfig::nucleus(0.9f, 0.8f);
        cfg.repetition_penalty = 1.2f;
        tests.push_back({"Combined (P=0.9, T=0.8, Pen=1.2)", cfg});
    }

    for (const auto& test : tests) {
        std::cout << "\n  --- Config: " << test.name << " ---" << std::endl;
        try {
            Generator generator(model, test.config);
            // Seed for reproducibility if needed, but for now let's see variance
            // generator.seed(42); 

            std::vector<int> generated_tokens = generator.generate(prompt_tokens, gen_len);
            std::string generated_text = tokenizer.decode(generated_tokens);
            std::cout << "  Generated: \"" << generated_text << "\"" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "  [ERROR] Generation failed: " << e.what() << std::endl;
        }
    }
    std::cout << "\n=== Generation Configuration Tests Completed ===" << std::endl;
}

void run_checkpoint_comparison() {
    std::cout << "=== Checkpoint Comparison Test ===" << std::endl;

    // 1. Setup Tokenizer
    // Try to load tokenizer from the training output directory first to ensure consistency
    fs::path project_root = fs::current_path().parent_path().parent_path().parent_path().parent_path();
    std::string training_dir = "quanta_tissu/tisslm/training_output";
    std::string tokenizer_path = (project_root / training_dir / "tokenizer").string();
    
    // Fallback to TestConfig if not found (though this might cause mismatch)
    if (!fs::exists(tokenizer_path + "_vocab.json")) {
        std::cout << "Tokenizer not found in training dir, using TestConfig path." << std::endl;
        tokenizer_path = TestConfig::TokenizerPath;
    }

    std::cout << "Loading tokenizer from: " << tokenizer_path << std::endl;
    Tokenizer tokenizer(tokenizer_path);
    int vocab_size = tokenizer.get_vocab_size();
    std::cout << "Tokenizer Vocab Size: " << vocab_size << std::endl;

    // Force vocab size to 5000 if it differs, as training was done with 5000
    // This handles cases where the tokenizer might have slightly fewer tokens but the model was initialized with 5000
    if (vocab_size != 5000) {
        std::cout << "WARNING: Tokenizer vocab size (" << vocab_size << ") != 5000. Forcing model vocab size to 5000 to match training." << std::endl;
        vocab_size = 5000;
    }

    // 2. Setup Model (Initial)
    std::cout << "Initializing model structure..." << std::endl;
    // Using same config as training (assumed from continue_training.cpp)
    // VOCAB_SIZE = 5000, SEQ_LEN = 128, EMBED_DIM = 128, HEADS = 4, LAYERS = 2, D_FF = 512
    int seq_len = 128;
    int embed_dim = 128;
    int num_heads = 4;
    int num_layers = 2;
    int d_ff = 512; 
    float dropout = 0.1f;

    auto model = std::make_shared<TransformerModel>(
        vocab_size, seq_len, embed_dim, num_heads, num_layers, d_ff, dropout
    );

    // 3. Setup Trainer (for loading checkpoints)
    auto loss_fn = std::make_shared<CrossEntropyLoss>();
    auto optimizer = std::make_shared<Adam>(1e-3f);
    Trainer trainer(model, optimizer, loss_fn);

    // 4. Checkpoints to test
    std::vector<std::string> checkpoints = {
        training_dir + "/checkpoint_epoch_1.pt",
        training_dir + "/checkpoint_epoch_5.pt",
        training_dir + "/checkpoint_epoch_10.pt",
        training_dir + "/checkpoint_epoch_15.pt",
        training_dir + "/final_model.pt"
    };

    // 5. Prompt
    std::string prompt_text = "The quick brown fox";
    std::cout << "Prompt: \"" << prompt_text << "\"" << std::endl;

    // 6. Iterate and Generate
    for (const auto& checkpoint_rel_path : checkpoints) {
        // Construct absolute path assuming we are in build dir
        // We need to find where training_output is. 
        // Usually it's in the project root.
        // The test binary runs in tests/model/program/build
        // So project root is ../../../..
        
        fs::path project_root = fs::current_path().parent_path().parent_path().parent_path().parent_path();
        fs::path checkpoint_path = project_root / checkpoint_rel_path;

        std::cout << "\n--- Testing Checkpoint: " << checkpoint_rel_path << " ---" << std::endl;
        
        if (!fs::exists(checkpoint_path)) {
            std::cout << "  [WARNING] Checkpoint not found at " << checkpoint_path << ". Skipping." << std::endl;
            continue;
        }

        try {
            trainer.load_checkpoint(checkpoint_path.string());
            std::cout << "  Checkpoint loaded successfully." << std::endl;

            // Generate
            GenerationConfig gen_config = GenerationConfig::greedy();
            Generator generator(model, gen_config);
            
            std::vector<int> prompt_tokens = tokenizer.encode(prompt_text);
            std::vector<int> generated_tokens = generator.generate(prompt_tokens, 50); // Generate 50 tokens
            std::string generated_text = tokenizer.decode(generated_tokens);

            std::cout << "  Generated: \"" << generated_text << "\"" << std::endl;

            // Run extended config tests for the final model
            if (checkpoint_rel_path.find("final_model.pt") != std::string::npos) {
                run_generation_config_tests(model, tokenizer);
            }

        } catch (const std::exception& e) {
            std::cerr << "  [ERROR] Failed to load or generate: " << e.what() << std::endl;
        }
    }

    std::cout << "\n=== Checkpoint Comparison Completed ===" << std::endl;
}

int main() {
    try {
        run_checkpoint_comparison();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

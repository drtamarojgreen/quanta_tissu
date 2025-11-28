#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <regex>
#include <filesystem>

#include "../tokenizer/tokenizer.h"
#include "../core/transformer_model.h"
#include "loss_function.h"
#include "optimizer.h"
#include "dataset.h"
#include "trainer.h"

namespace fs = std::filesystem;

// Helper to load corpus (copied from run_training.cpp)
std::string load_corpus(const std::string& path) {
    std::string content;
    if (fs::is_directory(path)) {
        std::cout << "Loading corpus from directory: " << path << std::endl;
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    content += buffer.str() + "\n";
                }
            }
        }
    } else {
        std::cout << "Loading corpus from file: " << path << std::endl;
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open corpus file " << path << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
    }
    return content;
}

int extract_epoch_from_filename(const std::string& filename) {
    std::regex re("checkpoint_epoch_(\\d+)\\.pt");
    std::smatch match;
    if (std::regex_search(filename, match, re)) {
        return std::stoi(match[1]);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <tokenizer_prefix> <checkpoint_path> <target_epochs> [corpus_path]" << std::endl;
        return 1;
    }

    std::string tokenizer_prefix = argv[1];
    std::string checkpoint_path = argv[2];
    int target_epochs = std::stoi(argv[3]);
    std::string corpus_path = (argc > 4) ? argv[4] : "corpus/corpus.txt";

    const std::string SAVE_DIR = "training_output";
    const int VOCAB_SIZE = 5000;
    const int SEQ_LEN = 128;
    const int BATCH_SIZE = 32;
    // const int TARGET_EPOCHS = 5; // Replaced by target_epochs variable
    const float LEARNING_RATE = 1e-3f;

    // 1. Load Corpus
    std::string corpus = load_corpus(corpus_path);
    if (corpus.empty()) return 1;

    // 2. Load Tokenizer
    std::cout << "Loading tokenizer from " << tokenizer_prefix << "..." << std::endl;
    auto tokenizer = std::make_unique<TissLM::Tokenizer::Tokenizer>(tokenizer_prefix);

    // 3. Create Dataset
    std::vector<int> token_ids = tokenizer->encode(corpus);
    if (token_ids.size() > 10000) {
        std::cout << "Limiting dataset to 10,000 tokens for verification." << std::endl;
        token_ids.resize(10000);
    }
    TissLM::Training::TokenDataset dataset(token_ids, SEQ_LEN);

    // 4. Create Model
    int d_ff = 128 * 4;
    auto model = std::make_shared<TissLM::Core::TransformerModel>(VOCAB_SIZE, SEQ_LEN, 128, 4, 2, d_ff, 0.1f);
    auto loss_fn = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    auto optimizer = std::make_shared<TissLM::Training::Adam>(LEARNING_RATE);

    TissLM::Training::Trainer trainer(model, optimizer, loss_fn);

    // 5. Load Checkpoint
    std::cout << "Loading checkpoint: " << checkpoint_path << std::endl;
    trainer.load_checkpoint(checkpoint_path);

    int start_epoch = extract_epoch_from_filename(checkpoint_path);
    if (start_epoch == 0) {
        std::cerr << "Warning: Could not extract epoch from filename. Assuming Epoch 0." << std::endl;
    } else {
        std::cout << "Resuming from end of Epoch " << start_epoch << " (Starting Epoch " << start_epoch + 1 << ")" << std::endl;
    }

    // 6. Resume Training
    for (int epoch = start_epoch; epoch < target_epochs; ++epoch) {
        std::cout << "Starting Epoch " << epoch + 1 << "/" << target_epochs << std::endl;
        trainer.train(dataset, 1, BATCH_SIZE);

        std::string new_checkpoint_path = SAVE_DIR + "/checkpoint_epoch_" + std::to_string(epoch + 1) + ".pt";
        trainer.save_checkpoint(new_checkpoint_path);
        std::cout << "Saved checkpoint: " << new_checkpoint_path << std::endl;
    }

    std::string final_model_path = SAVE_DIR + "/final_model.pt";
    trainer.save_checkpoint(final_model_path);
    std::cout << "Training complete. Final model saved: " << final_model_path << std::endl;

    // 7. Generate Text (Verification)
    std::string prompt = "The quick brown fox";
    std::cout << "\nGenerating text for prompt: '" << prompt << "'" << std::endl;
    std::vector<int> input_ids = tokenizer->encode(prompt);
    
    for (int i = 0; i < 20; ++i) {
        TissNum::Matrix input_mat({1, input_ids.size()});
        for(size_t j=0; j<input_ids.size(); ++j) input_mat({0, j}) = (float)input_ids[j];

        TissNum::Matrix logits = model->forward(input_mat, false);
        size_t last_token_idx = input_ids.size() - 1;
        float max_logit = -1e9;
        int best_token_id = 0;

        for (int v = 0; v < VOCAB_SIZE; ++v) {
            float val = logits({last_token_idx, (size_t)v});
            if (val > max_logit) {
                max_logit = val;
                best_token_id = v;
            }
        }

        input_ids.push_back(best_token_id);
        std::string decoded_token = tokenizer->decode({best_token_id});
        std::cout << "Debug: Step " << i << ", Best Token ID: " << best_token_id << ", Logit: " << max_logit << ", Decoded: '" << decoded_token << "'" << std::endl;
    }
    std::cout << std::endl;

    return 0;
}

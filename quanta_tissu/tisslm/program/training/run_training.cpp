#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>

#include "../tokenizer/tokenizer.h"
#include "../core/transformer_model.h"
#include "loss_function.h"
#include "optimizer.h"
#include "dataset.h"
#include "trainer.h"

#include <filesystem>

namespace fs = std::filesystem;

std::string load_corpus(const std::string& path) {
    std::string content;
    
    std::cout << "Debug: Current working directory: " << fs::current_path() << std::endl;
    std::cout << "Debug: Input path: " << path << std::endl;
    try {
        std::cout << "Debug: Absolute path: " << fs::absolute(path) << std::endl;
        std::cout << "Debug: Exists: " << (fs::exists(path) ? "Yes" : "No") << std::endl;
        std::cout << "Debug: Is directory: " << (fs::is_directory(path) ? "Yes" : "No") << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    if (fs::is_directory(path)) {
        std::cout << "Loading corpus from directory: " << path << std::endl;
        int file_count = 0;
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    content += buffer.str() + "\n"; // Add newline between files
                    file_count++;
                } else {
                    std::cerr << "Warning: Could not open file " << entry.path() << std::endl;
                }
            }
        }
        std::cout << "Loaded " << file_count << " files from " << path << std::endl;
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
    
    std::cout << "Total corpus size: " << content.size() << " bytes" << std::endl;
    return content;
}

int main(int argc, char* argv[]) {
    std::string CORPUS_PATH = "corpus/corpus.txt"; // Default
    if (argc > 1) {
        CORPUS_PATH = argv[1];
    }

    const std::string SAVE_DIR = "training_output";
    const int VOCAB_SIZE = 5000;
    const int SEQ_LEN = 128;
    const int BATCH_SIZE = 32;
    const int EPOCHS = 1; // Reduced for verification
    const float LEARNING_RATE = 1e-3f;

    // 1. Load corpus
    std::string corpus = load_corpus(CORPUS_PATH);
    if (corpus.empty()) {
        std::cerr << "Error: Corpus is empty or could not be loaded from " << CORPUS_PATH << std::endl;
        return 1;
    }

    // 2. Train or Load tokenizer
    std::unique_ptr<TissLM::Tokenizer::Tokenizer> tokenizer;
    std::string tokenizer_prefix = SAVE_DIR + "/tokenizer";
    
    if (fs::exists(tokenizer_prefix + "_vocab.json")) {
        std::cout << "Loading existing tokenizer from " << tokenizer_prefix << std::endl;
        tokenizer = std::make_unique<TissLM::Tokenizer::Tokenizer>(tokenizer_prefix);
    } else {
        std::cout << "Training tokenizer..." << std::endl;
        tokenizer = std::make_unique<TissLM::Tokenizer::Tokenizer>("");
        tokenizer->train(corpus, VOCAB_SIZE, true);
        // Create save directory
        std::string mkdir_cmd = "mkdir -p " + SAVE_DIR;
        system(mkdir_cmd.c_str());
        tokenizer->save(tokenizer_prefix);
    }

    // 3. Create dataset
    std::vector<int> token_ids = tokenizer->encode(corpus);
    
    // Limit dataset size for verification purposes
    if (token_ids.size() > 10000) {
        std::cout << "Limiting dataset to 10,000 tokens for verification." << std::endl;
        token_ids.resize(10000);
    }

    TissLM::Training::TokenDataset dataset(token_ids, SEQ_LEN);

    // 4. Create model, loss, and optimizer
    // TransformerModel(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, d_ff, dropout_rate)
    int d_ff = 128 * 4; // Usually 4 * embed_dim
    auto model = std::make_shared<TissLM::Core::TransformerModel>(VOCAB_SIZE, SEQ_LEN, 128, 4, 2, d_ff, 0.1f);
    auto loss_fn = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    auto optimizer = std::make_shared<TissLM::Training::Adam>(LEARNING_RATE);

    // 5. Create trainer and train
    TissLM::Training::Trainer trainer(model, optimizer, loss_fn);
    
    for (int epoch = 0; epoch < EPOCHS; ++epoch) {
        std::cout << "Starting Epoch " << epoch + 1 << "/" << EPOCHS << std::endl;
        trainer.train(dataset, 1, BATCH_SIZE);

        std::string checkpoint_path = SAVE_DIR + "/checkpoint_epoch_" + std::to_string(epoch + 1) + ".pt";
        trainer.save_checkpoint(checkpoint_path);
        std::cout << "Saved checkpoint: " << checkpoint_path << std::endl;
    }

    std::string final_model_path = SAVE_DIR + "/final_model.pt";
    trainer.save_checkpoint(final_model_path);
    std::cout << "Training complete. Final model saved: " << final_model_path << std::endl;

    // 6. Generate text
    std::string prompt = "The quick brown fox";
    std::cout << "\nGenerating text for prompt: '" << prompt << "'" << std::endl;
    std::vector<int> input_ids = tokenizer->encode(prompt);

    for (int i = 0; i < 20; ++i) {
        TissNum::Matrix input_mat({1, input_ids.size()});
        for(size_t j=0; j<input_ids.size(); ++j) {
            input_mat({0, j}) = (float)input_ids[j];
        }

        TissNum::Matrix logits = model->forward(input_mat);

        size_t last_token_idx = input_ids.size() - 1;
        float max_logit = -1e9;
        int best_token_id = 0;

        for (int v = 0; v < VOCAB_SIZE; ++v) {
            // Logits are 2D: [seq_len, vocab_size] because forward() removes batch dim
            float val = logits({last_token_idx, (size_t)v});
            if (val > max_logit) {
                max_logit = val;
                best_token_id = v;
            }
        }

        input_ids.push_back(best_token_id);
        std::cout << tokenizer->decode({best_token_id}) << std::flush;
    }
    std::cout << std::endl;

    return 0;
}

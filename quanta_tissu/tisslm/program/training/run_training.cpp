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

std::string load_corpus(const std::string& corpus_path) {
    std::ifstream file(corpus_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open corpus file " << corpus_path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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
    const int EPOCHS = 5;
    const float LEARNING_RATE = 1e-3f;

    // 1. Load corpus
    std::string corpus = load_corpus(CORPUS_PATH);
    if (corpus.empty()) {
        std::cerr << "Error: Corpus is empty or could not be loaded from " << CORPUS_PATH << std::endl;
        return 1;
    }

    // 2. Train tokenizer
    TissLM::Tokenizer::Tokenizer tokenizer("");
    tokenizer.train(corpus, VOCAB_SIZE, true);
    // Create save directory
    std::string mkdir_cmd = "mkdir -p " + SAVE_DIR;
    system(mkdir_cmd.c_str());
    tokenizer.save(SAVE_DIR + "/tokenizer");

    // 3. Create dataset
    std::vector<int> token_ids = tokenizer.encode(corpus);
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
    std::vector<int> input_ids = tokenizer.encode(prompt);

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
        std::cout << tokenizer.decode({best_token_id}) << std::flush;
    }
    std::cout << std::endl;

    return 0;
}

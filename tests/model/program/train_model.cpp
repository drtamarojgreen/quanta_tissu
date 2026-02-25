#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include "quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "quanta_tissu/tisslm/program/training/trainer.h"
#include "quanta_tissu/tisslm/program/training/dataset.h"
#include "quanta_tissu/tisslm/program/training/optimizer.h"
#include "quanta_tissu/tisslm/program/training/loss_function.h"
#include "config/TestConfig.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <glob.h>
#include <cstring>
#include <filesystem>

// Function to load corpus from a directory
std::string load_corpus(const std::string& path) {
    std::stringstream corpus_stream;
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    std::string pattern = path + "/*.txt";
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        std::cerr << "glob() failed with return_value " << return_value << std::endl;
        return "";
    }

    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        std::ifstream file(glob_result.gl_pathv[i]);
        if (file) {
            corpus_stream << file.rdbuf();
            file.close();
        }
    }
    globfree(&glob_result);
    return corpus_stream.str();
}

void run_training() {
    std::cout << "=== Running C++ Model Training ===" << std::endl;

    // --- 1. Load Corpus ---
    std::cout << "[1/5] Loading corpus..." << std::endl;
    std::string corpus = load_corpus("./corpus");
    if (corpus.empty()) {
        std::cerr << "Failed to load corpus. Exiting." << std::endl;
        return;
    }
    std::cout << "Corpus loaded." << std::endl;

    // --- 2. Train Tokenizer ---
    std::cout << "[2/5] Training tokenizer..." << std::endl;
    auto tokenizer = std::make_shared<TissLM::Tokenizer::Tokenizer>("");
    tokenizer->train(corpus, 4196);
    tokenizer->save("trained_tokenizer");
    std::cout << "Tokenizer trained and saved." << std::endl;

    // --- 3. Create Dataset ---
    std::cout << "[3/5] Creating dataset..." << std::endl;
    std::vector<int> token_ids = tokenizer->encode(corpus);
    TissLM::Training::TokenDataset dataset(token_ids, 1024);
    std::cout << "Dataset created." << std::endl;

    // --- 4. Initialize Training Components ---
    std::cout << "[4/5] Initializing training components..." << std::endl;
    int vocab_size = tokenizer->get_vocab_size();
    auto model = std::make_shared<TissLM::Core::TransformerModel>(
        vocab_size,
        1024, // max_seq_len
        768,  // embed_dim
        12,   // num_heads
        12,   // num_layers
        0.1f, // dropout_rate
        4     // lora_rank
    );
    auto optimizer = std::make_shared<TissLM::Training::Adam>(1e-4);
    auto loss_function = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    std::cout << "Training components initialized." << std::endl;

    // --- 5. Run Training ---
    std::cout << "[5/5] Starting training..." << std::endl;
    const std::string checkpoint_dir = "checkpoints";
    std::filesystem::create_directories(checkpoint_dir);
    TissLM::Training::Trainer trainer(model, optimizer, loss_function);
    trainer.train(dataset, 3, 1, 100, checkpoint_dir);
    std::cout << "Training completed." << std::endl;
}

int main() {
    try {
        run_training();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C++ Training failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

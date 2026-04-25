#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include "quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "quanta_tissu/tisslm/program/training/trainer.h"
#include "quanta_tissu/tisslm/program/training/dataset.h"
#include "quanta_tissu/tisslm/program/training/optimizer.h"
#include "quanta_tissu/tisslm/program/training/loss_function.h"
// #include "config/TestConfig.h" // Not strictly needed if not using TestConfig defines, but good to keep consistent

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <glob.h> // For glob function
#include <cstring> // For memset

// Function to load corpus from a directory
std::string load_corpus_for_dataset(const std::string& path) {
    std::stringstream corpus_stream;
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    std::string pattern = path + "/*.txt";
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        // It's possible for glob to return GLOB_NOMATCH if no files match, which is not an error in all contexts
        // but here it means no corpus was found, which is an error for dataset creation.
        if (return_value != GLOB_NOMATCH) {
            std::cerr << "glob() failed with return_value " << return_value << std::endl;
        } else {
            std::cerr << "No corpus files found in " << path << std::endl;
        }
        return "";
    }

    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        std::ifstream file(glob_result.gl_pathv[i]);
        if (file) {
            corpus_stream << file.rdbuf();
            file.close();
        } else {
            std::cerr << "Could not open file: " << glob_result.gl_pathv[i] << std::endl;
        }
    }
    globfree(&glob_result);
    return corpus_stream.str();
}

void run_resume_training() {
    std::cout << "=== Running C++ Model Training Resumption ===" << std::endl;

    // --- 1. Load Tokenizer --- (Corresponds to starting from original Step 3 preparation)
    std::cout << "[1/4] Loading pre-trained tokenizer..." << std::endl;
    auto tokenizer = std::make_shared<TissLM::Tokenizer::Tokenizer>("trained_tokenizer");
    std::cout << "Tokenizer loaded." << std::endl;

    // --- 2. Create Dataset --- (Corresponds to original Step 3)
    std::cout << "[2/4] Creating dataset..." << std::endl;
    std::string corpus = load_corpus_for_dataset("./corpus");
    if (corpus.empty()) {
        std::cerr << "Failed to load corpus for dataset creation. Exiting." << std::endl;
        return;
    }
    std::vector<int> token_ids = tokenizer->encode(corpus);
    TissLM::Training::TokenDataset dataset(token_ids, 128);
    std::cout << "Dataset created." << std::endl;

    // --- 3. Initialize Training Components --- (Corresponds to original Step 4)
    std::cout << "[3/4] Initializing training components..." << std::endl;
    int vocab_size = tokenizer->get_vocab_size();
    auto model = std::make_shared<TissLM::Core::TransformerModel>(
        vocab_size,
        128, // max_seq_len
        128,  // embed_dim
        2,    // num_heads
        2,    // num_layers
        0.1f, // dropout_rate
        1     // lora_rank
    );
    auto optimizer = std::make_shared<TissLM::Training::Adam>(1e-4);
    auto loss_function = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    std::cout << "Training components initialized." << std::endl;

    // --- 4. Run Training --- (Corresponds to original Step 5)
    std::cout << "[4/4] Starting training..." << std::endl;
    const std::string checkpoint_dir = "checkpoints";
    std::filesystem::create_directories(checkpoint_dir);
    TissLM::Training::Trainer trainer(model, optimizer, loss_function);
    trainer.train(dataset, 1, 64, 10, checkpoint_dir);
    std::cout << "Training completed." << std::endl;
}

int main() {
    try {
        run_resume_training();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C++ Training failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

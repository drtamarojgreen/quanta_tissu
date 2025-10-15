#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

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

int main() {
    const std::string CORPUS_PATH = "corpus/corpus.txt"; // Assuming a single corpus file
    const std::string SAVE_DIR = "training_output";
    const int VOCAB_SIZE = 5000;
    const int SEQ_LEN = 128;
    const int BATCH_SIZE = 32;
    const int EPOCHS = 5;
    const float LEARNING_RATE = 1e-3f;

    // 1. Load corpus
    std::string corpus = load_corpus(CORPUS_PATH);
    if (corpus.empty()) {
        return 1;
    }

    // 2. Train tokenizer
    Tokenizer tokenizer("");
    tokenizer.train(corpus, VOCAB_SIZE, true);
    tokenizer.save(SAVE_DIR + "/tokenizer");

    // 3. Create dataset
    std::vector<int> token_ids = tokenizer.encode(corpus);
    TissDB::TissLM::Training::TokenDataset dataset(token_ids, SEQ_LEN);

    // 4. Create model, loss, and optimizer
    auto model = std::make_shared<TissDB::TissLM::Core::TransformerModel>(VOCAB_SIZE, SEQ_LEN, 128, 4, 2, 0.1f);
    auto loss_fn = std::make_shared<TissDB::TissLM::Core::CrossEntropyLoss>();
    auto optimizer = std::make_shared<TissDB::TissLM::Core::Adam>(LEARNING_RATE);

    // 5. Create trainer and train
    TissDB::TissLM::Core::Trainer trainer(model, optimizer, loss_fn);
    trainer.train(dataset, EPOCHS, BATCH_SIZE);

    return 0;
}

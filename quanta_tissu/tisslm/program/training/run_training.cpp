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
#include "../config/model_config.h"
#include "../pipelines/config/training_config.h"

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
    // 1. Load corpus
    std::string corpus = load_corpus(TissDB::TissLM::Config::CORPUS_PATH);
    if (corpus.empty()) {
        return 1;
    }

    // 2. Train tokenizer
    Tokenizer tokenizer("");
    tokenizer.train(corpus, TissDB::TissLM::Config::VOCAB_SIZE, true);
    tokenizer.save(TissDB::TissLM::Config::SAVE_DIR + "/tokenizer");

    // 3. Create dataset
    std::vector<int> token_ids = tokenizer.encode(corpus);
    TissDB::TissLM::Training::TokenDataset dataset(token_ids, TissDB::TissLM::Config::SEQ_LEN);

    // 4. Create model, loss, and optimizer
    auto model = std::make_shared<TissDB::TissLM::Core::TransformerModel>(
        TissDB::TissLM::Config::VOCAB_SIZE,
        TissDB::TissLM::Config::SEQ_LEN,
        TissDB::TissLM::Config::EMBED_DIM,
        TissDB::TissLM::Config::NUM_HEADS,
        TissDB::TissLM::Config::NUM_LAYERS,
        TissDB::TissLM::Config::DROPOUT_RATE
    );
    auto loss_fn = std::make_shared<TissDB::TissLM::Core::CrossEntropyLoss>();
    auto optimizer = std::make_shared<TissDB::TissLM::Core::Adam>(TissDB::TissLM::Config::LEARNING_RATE);

    // 5. Create trainer and train
    TissDB::TissLM::Core::Trainer trainer(model, optimizer, loss_fn);
    trainer.train(dataset, TissDB::TissLM::Config::EPOCHS, TissDB::TissLM::Config::BATCH_SIZE);

    return 0;
}

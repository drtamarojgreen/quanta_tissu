#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/training/trainer.h"
#include "../../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../../quanta_tissu/tisslm/program/training/loss_function.h"
#include "../../../quanta_tissu/tisslm/program/training/dataset.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <numeric>


using namespace TissDB::TissLM::Core;
using namespace TissDB::TissLM::Training;
using namespace TissNum;

// Helper to check if a file exists
bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

// Basic checkpoint save function (since it's not in the Trainer class)
void save_checkpoint(const std::string& path, std::shared_ptr<Model> model) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }
    auto params = model->get_parameters();
    for (const auto& param_ptr : params) {
        const auto& param = *param_ptr;
        const auto& value_matrix = param.value();
        ofs.write(reinterpret_cast<const char*>(value_matrix.get_data()), value_matrix.rows() * value_matrix.cols() * sizeof(float));
    }
}


void test_checkpointing() {
    std::cout << "=== Testing Model Checkpointing ===" << std::endl;

    // --- 1. Setup Model, Optimizer, and Data ---
    int vocab_size = 100; // Small vocab for test
    auto model = std::make_shared<TransformerModel>(
        vocab_size,
        TestConfig::MaxSeqLen,
        TestConfig::EmbedDim,
        TestConfig::NumHeads,
        TestConfig::NumLayers,
        TestConfig::FFNDim,
        TestConfig::DropoutRate
    );

    auto optimizer = std::make_shared<Adam>(0.001f);
    auto loss_fn = std::make_shared<CrossEntropyLoss>();

    // Dummy data
    std::vector<int> token_data;
    for(int i = 0; i < 10; ++i) token_data.push_back(i);
    TokenDataset dataset(token_data, 5);


    Trainer trainer(model, optimizer, loss_fn);

    // --- 2. Run a few training steps ---
    std::cout << "  Running a few training steps..." << std::endl;
    trainer.train(dataset, 2, 1); // 2 epochs, batch size 1

    // --- 3. Save a checkpoint ---
    std::string checkpoint_path = "test_checkpoint.bin";
    std::cout << "  Saving checkpoint to: " << checkpoint_path << std::endl;
    save_checkpoint(checkpoint_path, model);

    // --- 4. Verify checkpoint file exists ---
    if (file_exists(checkpoint_path)) {
        std::cout << "  [PASSED] Checkpoint file was created successfully." << std::endl;
    } else {
        std::cout << "  [FAILED] Checkpoint file was not created." << std::endl;
        throw std::runtime_error("Checkpoint file creation failed.");
    }

    // --- 5. (Optional) Load checkpoint and verify parameters ---
    // This would require more extensive parameter comparison logic.
    // For now, we confirm creation.

    std::cout << "Checkpointing test completed successfully." << std::endl;
}

int main() {
    try {
        test_checkpointing();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Checkpointing test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

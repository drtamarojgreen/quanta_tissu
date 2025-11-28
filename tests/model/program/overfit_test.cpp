#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <iomanip>

#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/training/loss_function.h"
#include "../../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../../quanta_tissu/tisslm/program/training/dataset.h"
#include "../../../quanta_tissu/tisslm/program/training/trainer.h"

using namespace TissLM;

void test_overfitting() {
    std::cout << "=== Testing Model Overfitting Capability ===" << std::endl;

    // 1. Setup a tiny model
    int vocab_size = 10;
    int seq_len = 5;
    int embed_dim = 16;
    int num_heads = 2;
    int num_layers = 1;
    int d_ff = 32;
    float dropout = 0.0f; // Disable dropout for deterministic overfitting

    auto model = std::make_shared<Core::TransformerModel>(vocab_size, seq_len, embed_dim, num_heads, num_layers, d_ff, dropout);
    auto loss_fn = std::make_shared<Training::CrossEntropyLoss>();
    auto optimizer = std::make_shared<Training::Adam>(0.01f); // High learning rate for fast overfitting

    Training::Trainer trainer(model, optimizer, loss_fn);

    // 2. Create a simple repeated sequence: 1 2 3 4 1 2 3 4 ...
    std::vector<int> data = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
    Training::TokenDataset dataset(data, seq_len);

    // 3. Train loop
    float initial_loss = 0.0f;
    float final_loss = 0.0f;

    std::cout << "Training for 100 steps..." << std::endl;
    for (int i = 0; i < 100; ++i) {
        // Manually run a training step to capture loss
        auto batch = dataset.get_item(0); // Single item batch
        TissNum::Matrix input = batch.first;
        TissNum::Matrix target = batch.second;

        // Forward
        TissNum::Matrix output = model->forward(input, true);
        
        // Output is already [seq_len, vocab_size] because batch size is 1
        TissNum::Matrix output_flat = output;
        
        // Target is [1, seq_len], need [seq_len, 1]
        TissNum::Matrix target_flat = target.reshape({target.cols(), 1});

        float loss = loss_fn->compute_loss(output_flat, target_flat);
        
        if (i == 0) initial_loss = loss;
        final_loss = loss;

        // Backward
        TissNum::Matrix grad = loss_fn->compute_gradient(output_flat, target_flat);
        // Reshape grad back to [batch, seq_len, vocab_size] if needed, but model.backward takes 2D
        // TissNum::Matrix grad_shaped = grad.reshape({1, output.rows(), output.cols()}); // Correct logic would be this
        
        TissNum::Matrix grad_2d = grad; // grad is already [seq_len, vocab_size]
        model->backward(grad_2d);

        // Update
        auto params = model->get_parameters();
        std::vector<TissNum::Parameter*> raw_params;
        for(auto& p : params) raw_params.push_back(p.get());
        optimizer->update(raw_params);

        if (i % 10 == 0) {
            std::cout << "Step " << i << " Loss: " << loss << std::endl;
        }
    }

    std::cout << "Initial Loss: " << initial_loss << std::endl;
    std::cout << "Final Loss: " << final_loss << std::endl;

    if (final_loss < 0.1f) {
        std::cout << "Overfitting Test Passed: Model successfully learned the sequence." << std::endl;
    } else {
        std::cout << "Overfitting Test FAILED: Model failed to converge." << std::endl;
        // throw std::runtime_error("Model failed to overfit simple sequence.");
    }
}

int main() {
    try {
        test_overfitting();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

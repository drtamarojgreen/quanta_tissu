#include "trainer.h"
#include <random>
#include <chrono>
#include <algorithm> // For std::shuffle

#include "trainer.h"
#include <random>
#include <chrono>
#include <algorithm> // For std::shuffle

namespace TissLM {
namespace Training {

using namespace TissLM::Core;
using namespace TissNum;

Trainer::Trainer(
    std::shared_ptr<Core::Model> model,
    std::shared_ptr<Optimizer> optimizer,
    std::shared_ptr<LossFunction> loss_function
) : model_(model), optimizer_(optimizer), loss_function_(loss_function) {
}

void Trainer::train(
    TokenDataset& dataset,
    int epochs,
    int batch_size
) {
    int num_samples = dataset.size();
    if (num_samples == 0) {
        std::cerr << "No training data provided." << std::endl;
        return;
    }

    std::vector<int> indices(num_samples);
    std::iota(indices.begin(), indices.end(), 0);

    std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

    for (int epoch = 0; epoch < epochs; ++epoch) {
        std::shuffle(indices.begin(), indices.end(), rng);
        float epoch_loss = 0.0f;
        int num_batches = (num_samples + batch_size - 1) / batch_size;

        for (int b = 0; b < num_batches; ++b) {
            int batch_start = b * batch_size;
            int batch_end = std::min(batch_start + batch_size, num_samples);
            int current_batch_size = batch_end - batch_start;

            TissNum::Matrix batch_input(current_batch_size, dataset.get_item(0).first.cols());
            TissNum::Matrix batch_target(current_batch_size, dataset.get_item(0).second.cols());

            for (int i = 0; i < current_batch_size; ++i) {
                int sample_idx = indices[batch_start + i];
                auto item = dataset.get_item(sample_idx);
                for (int col = 0; col < item.first.cols(); ++col) {
                    batch_input(i, col) = item.first(0, col);
                }
                for (int col = 0; col < item.second.cols(); ++col) {
                    batch_target(i, col) = item.second(0, col);
                }
            }

            // Forward pass
            TissNum::Matrix predictions = model_->forward(batch_input);

            // Reshape target for loss computation
            TissNum::Matrix reshaped_target(batch_target.rows() * batch_target.cols(), 1);
            for (int r = 0; r < batch_target.rows(); ++r) {
                for (int c = 0; c < batch_target.cols(); ++c) {
                    reshaped_target(r * batch_target.cols() + c, 0) = batch_target(r, c);
                }
            }

            // Compute loss
            float loss = loss_function_->compute_loss(predictions, reshaped_target);
            epoch_loss += loss;

            // Compute gradient of loss w.r.t. predictions
            TissNum::Matrix grad_loss = loss_function_->compute_gradient(predictions, reshaped_target);

            // Backward pass
            model_->backward(grad_loss);

            // Update parameters
            std::vector<TissNum::Parameter*> params = model_->get_parameters();
            optimizer_->update(params);
        }
        std::cout << "Epoch " << epoch + 1 << ", Loss: " << epoch_loss / num_batches << std::endl;
    }
}

} // namespace Training
} // namespace TissLM

#include "trainer.h"
#include <random>
#include <chrono>

namespace TissDB {
namespace TissLM {
namespace Core {

Trainer::Trainer(
    std::shared_ptr<Model> model,
    std::shared_ptr<Optimizer> optimizer,
    std::shared_ptr<LossFunction> loss_function
) : model_(model), optimizer_(optimizer), loss_function_(loss_function) {
}

void Trainer::train(
    const std::vector<Matrix>& train_data,
    const std::vector<Matrix>& train_labels,
    int epochs,
    int batch_size
) {
    int num_samples = train_data.size();
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
            // Prepare batch data
            int batch_start = b * batch_size;
            int batch_end = std::min(batch_start + batch_size, num_samples);
            int current_batch_size = batch_end - batch_start;

            // Assuming input data is 2D (batch_size x input_dim)
            // And labels are 2D (batch_size x output_dim)
            // This part needs to be adapted based on actual data structure
            // For now, let's assume a simple concatenation for batching
            // This is a placeholder and might need more sophisticated batching logic
            Matrix batch_input(current_batch_size, train_data[0].cols());
            Matrix batch_target(current_batch_size, train_labels[0].cols());

            for (int i = 0; i < current_batch_size; ++i) {
                int sample_idx = indices[batch_start + i];
                // Copy row by row
                for (int col = 0; col < train_data[sample_idx].cols(); ++col) {
                    batch_input.set(i, col, train_data[sample_idx].get(0, col));
                }
                for (int col = 0; col < train_labels[sample_idx].cols(); ++col) {
                    batch_target.set(i, col, train_labels[sample_idx].get(0, col));
                }
            }

            // Forward pass
            Matrix predictions = model_->forward(batch_input);

            // Compute loss
            float loss = loss_function_->compute_loss(predictions, batch_target);
            epoch_loss += loss;

            // Compute gradient of loss w.r.t. predictions
            Matrix grad_loss = loss_function_->compute_gradient(predictions, batch_target);

            // Backward pass
            model_->backward(grad_loss);

            // Update parameters
            std::vector<std::shared_ptr<Parameter>> params = model_->get_parameters();
            optimizer_->update(params);
        }
        std::cout << "Epoch " << epoch + 1 << ", Loss: " << epoch_loss / num_batches << std::endl;
    }
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

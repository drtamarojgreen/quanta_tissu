#include "evaluator.h"
#include <iostream>
#include <numeric>

namespace TissDB {
namespace TissLM {
namespace Core {

Evaluator::Evaluator(
    std::shared_ptr<Model> model,
    std::shared_ptr<LossFunction> loss_function
) : model_(model), loss_function_(loss_function) {
}

float Evaluator::calculate_perplexity(
    const std::vector<Matrix>& eval_data,
    const std::vector<Matrix>& eval_labels,
    int batch_size
) {
    int num_samples = eval_data.size();
    if (num_samples == 0) {
        std::cerr << "No evaluation data provided." << std::endl;
        return -1.0f; // Indicate error or no data
    }

    float total_loss = 0.0f;
    int num_batches = (num_samples + batch_size - 1) / batch_size;

    for (int b = 0; b < num_batches; ++b) {
        int batch_start = b * batch_size;
        int batch_end = std::min(batch_start + batch_size, num_samples);
        int current_batch_size = batch_end - batch_start;

        Matrix batch_input({(size_t)current_batch_size, eval_data[0].cols()});
        Matrix batch_target({(size_t)current_batch_size, eval_labels[0].cols()});

        for (int i = 0; i < current_batch_size; ++i) {
            int sample_idx = batch_start + i;
            for (size_t col = 0; col < eval_data[sample_idx].cols(); ++col) {
                batch_input({(size_t)i, col}) = eval_data[sample_idx]({0, col});
            }
            for (size_t col = 0; col < eval_labels[sample_idx].cols(); ++col) {
                batch_target({(size_t)i, col}) = eval_labels[sample_idx]({0, col});
            }
        }

        Matrix predictions = model_->forward(batch_input);
        total_loss += loss_function_->compute_loss(predictions, batch_target);
    }

    float average_loss = total_loss / num_batches;
    return std::exp(average_loss);
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB

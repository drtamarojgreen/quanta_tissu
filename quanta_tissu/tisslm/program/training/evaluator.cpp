#include "evaluator.h"
#include <iostream>
#include <numeric>
#include <cmath>

namespace TissLM {
namespace Training {

Evaluator::Evaluator(
    std::shared_ptr<TissLM::Core::Model> model,
    std::shared_ptr<TissLM::Training::LossFunction> loss_function
) : model_(model), loss_function_(loss_function) {}

float Evaluator::calculate_perplexity(
    const std::vector<TissNum::Matrix>& eval_data,
    const std::vector<TissNum::Matrix>& eval_labels,
    int batch_size
) {
    if (eval_data.empty()) return -1.0f;
    float total_loss = 0.0f;
    int num_samples = (int)eval_data.size();
    int num_batches = (num_samples + batch_size - 1) / batch_size;

    for (int b = 0; b < num_batches; ++b) {
        int start = b * batch_size, end = std::min(start + batch_size, num_samples), cur_bz = end - start;
        TissNum::Matrix in({(size_t)cur_bz, eval_data[0].cols()}), tgt({(size_t)cur_bz, eval_labels[0].cols()});
        for (int i = 0; i < cur_bz; ++i) {
            for (size_t c = 0; c < eval_data[0].cols(); ++c) in({(size_t)i, c}) = eval_data[start + i]({0, c});
            for (size_t c = 0; c < eval_labels[0].cols(); ++c) tgt({(size_t)i, c}) = eval_labels[start + i]({0, c});
        }
        TissNum::Matrix pred = model_->forward(in);
        total_loss += loss_function_->compute_loss(pred, tgt);
    }
    return std::exp(total_loss / (float)num_batches);
}

}
}

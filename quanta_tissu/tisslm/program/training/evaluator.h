#ifndef TISSLM_EVALUATOR_H
#define TISSLM_EVALUATOR_H

#include "core/model_interface.h"
#include "loss_function.h"

#include <memory>
#include <vector>
#include <cmath>

namespace TissLM {
namespace Training {

class Evaluator {
public:
    Evaluator(
        std::shared_ptr<TissLM::Core::Model> model,
        std::shared_ptr<TissLM::Training::LossFunction> loss_function
    );

    float calculate_perplexity(
        const std::vector<TissNum::Matrix>& eval_data,
        const std::vector<TissNum::Matrix>& eval_labels,
        int batch_size
    );

private:
    std::shared_ptr<TissLM::Core::Model> model_;
    std::shared_ptr<TissLM::Training::LossFunction> loss_function_;
};

} // namespace Training
} // namespace TissLM

#endif // TISSLM_EVALUATOR_H

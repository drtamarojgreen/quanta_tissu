#ifndef TISSLM_EVALUATOR_H
#define TISSLM_EVALUATOR_H

#include "../../core/model_interface.h"
#include "loss_function.h"

#include <memory>
#include <vector>
#include <cmath>

namespace TissDB {
namespace TissLM {
namespace Core {

class Evaluator {
public:
    Evaluator(
        std::shared_ptr<Model> model,
        std::shared_ptr<LossFunction> loss_function
    );

    float calculate_perplexity(
        const std::vector<Matrix>& eval_data,
        const std::vector<Matrix>& eval_labels,
        int batch_size
    );

private:
    std::shared_ptr<Model> model_;
    std::shared_ptr<LossFunction> loss_function_;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_EVALUATOR_H

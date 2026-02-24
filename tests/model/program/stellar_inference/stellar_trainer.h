#ifndef TISSLM_STELLAR_TRAINER_H
#define TISSLM_STELLAR_TRAINER_H

#include "core/model_interface.h"
#include "training/optimizer.h"
#include "training/loss_function.h"
#include "training/dataset.h"
#include "stellar_visualizer.h"
#include <memory>
#include <vector>

namespace TissLM {
namespace Stellar {

/**
 * @class StellarTrainer
 * @brief Full implementation of the TissLM Training loop with Stellar enhancements.
 */
class StellarTrainer {
public:
    StellarTrainer(
        std::shared_ptr<TissLM::Core::Model> model,
        std::shared_ptr<TissLM::Training::Optimizer> optimizer,
        std::shared_ptr<TissLM::Training::LossFunction> loss_function
    );

    /**
     * @brief High-performance training loop with 3D ASCII visualization.
     */
    void train_stellar(
        TissLM::Training::TokenDataset& dataset,
        int epochs,
        int batch_size,
        bool visualize = true
    );

private:
    std::shared_ptr<TissLM::Core::Model> model_;
    std::shared_ptr<TissLM::Training::Optimizer> optimizer_;
    std::shared_ptr<TissLM::Training::LossFunction> loss_function_;
    std::vector<Point3D> loss_history_;
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_TRAINER_H

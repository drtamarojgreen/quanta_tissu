#ifndef TISSLM_STELLAR_TRAINER_H
#define TISSLM_STELLAR_TRAINER_H

/**
 * @file stellar_trainer.h
 * @brief Stellar wrapper for the existing TissLM Trainer.
 */

// Accessing internals for stellar visualization without modifying production code
#define private public
#define protected public
#include "training/trainer.h"
#undef private
#undef protected

#include "stellar_visualizer.h"
#include <vector>
#include <memory>

namespace TissLM {
namespace Stellar {

/**
 * @class StellarTrainer
 * @brief Extends the existing Trainer to provide Stellar 3D telemetry.
 */
class StellarTrainer : public TissLM::Training::Trainer {
public:
    using TissLM::Training::Trainer::Trainer; // Inherit existing constructors

    /**
     * @brief Trains the model using the existing engine while generating 3D telemetry.
     */
    void train_stellar(
        TissLM::Training::TokenDataset& dataset,
        int epochs,
        int batch_size,
        bool visualize = true
    );

private:
    std::vector<Point3D> loss_history_;
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_TRAINER_H

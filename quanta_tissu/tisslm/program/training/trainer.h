#ifndef TISSLM_TRAINER_H
#define TISSLM_TRAINER_H

#include "core/model_interface.h"
#include "optimizer.h"
#include "loss_function.h"

#include "dataset.h"

#include <memory>
#include <vector>
#include <iostream>

namespace TissDB {
namespace TissLM {
namespace Core {

class Trainer {
public:
    Trainer(
        std::shared_ptr<Model> model,
        std::shared_ptr<Optimizer> optimizer,
        std::shared_ptr<LossFunction> loss_function
    );

    void train(
        TissDB::TissLM::Training::TokenDataset& dataset, // Use TokenDataset
        int epochs,
        int batch_size
    );

private:
    std::shared_ptr<Model> model_;
    std::shared_ptr<Optimizer> optimizer_;
    std::shared_ptr<LossFunction> loss_function_;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_TRAINER_H

#ifndef TISSLM_TRAINER_H
#define TISSLM_TRAINER_H

#include "core/model_interface.h"
#include "optimizer.h"
#include "loss_function.h"

#include "dataset.h"

#include <memory>
#include <vector>
#include <iostream>

namespace TissLM {
namespace Training {

class Trainer {
public:
    Trainer(
        std::shared_ptr<Core::Model> model,
        std::shared_ptr<Optimizer> optimizer,
        std::shared_ptr<LossFunction> loss_function
    );

    void train(
        TokenDataset& dataset, // Use TokenDataset
        int epochs,
        int batch_size,
        int checkpoint_every_n_batches = 0,
        const std::string& checkpoint_dir = ""
    );

    void save_checkpoint(const std::string& path) const;
    void load_checkpoint(const std::string& path);

private:
    std::shared_ptr<Core::Model> model_;
    std::shared_ptr<Optimizer> optimizer_;
    std::shared_ptr<LossFunction> loss_function_;
};

} // namespace Training
} // namespace TissLM

#endif // TISSLM_TRAINER_H

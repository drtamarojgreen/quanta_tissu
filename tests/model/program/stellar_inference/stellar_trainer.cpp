#include "stellar_trainer.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <numeric>

namespace TissLM {
namespace Stellar {

using namespace TissLM::Training;
using namespace TissLM::Core;
using namespace TissNum;

void StellarTrainer::train_stellar(
    TokenDataset& dataset,
    int epochs,
    int batch_size,
    bool visualize
) {
    size_t num_samples = dataset.size();
    if (num_samples == 0) return;

    std::default_random_engine rng(42);
    std::vector<int> indices(num_samples);
    std::iota(indices.begin(), indices.end(), 0);

    std::cout << "[STELLAR] Training Loop: Delegating to core TissLM implementation..." << std::endl;
    loss_history_.clear();

    for (int epoch = 0; epoch < epochs; ++epoch) {
        std::shuffle(indices.begin(), indices.end(), rng);
        size_t num_batches = (num_samples + batch_size - 1) / batch_size;

        for (size_t b = 0; b < num_batches; ++b) {
            size_t batch_start = b * batch_size;
            size_t batch_end = std::min((size_t)batch_start + batch_size, num_samples);
            size_t current_batch_size = batch_end - batch_start;

            TissNum::Matrix batch_input({current_batch_size, dataset.get_item(0).first.cols()});
            TissNum::Matrix batch_target({current_batch_size, dataset.get_item(0).second.cols()});

            for (size_t i = 0; i < current_batch_size; ++i) {
                auto item = dataset.get_item(indices[batch_start + i]);
                for (size_t col = 0; col < item.first.cols(); ++col) batch_input({i, col}) = item.first({0, col});
                for (size_t col = 0; col < item.second.cols(); ++col) batch_target({i, col}) = item.second({0, col});
            }

            // --- DELEGATION TO EXISTING MODEL & LOSS ---
            TissNum::Matrix predictions = model_->forward(batch_input);
            TissNum::Matrix flat_target({batch_target.rows() * batch_target.cols(), 1});
            for (size_t r = 0; r < batch_target.rows(); ++r) {
                for (size_t c = 0; c < batch_target.cols(); ++c) {
                    flat_target({r * batch_target.cols() + c, 0}) = batch_target({r, c});
                }
            }

            float loss = loss_function_->compute_loss(predictions, flat_target);

            // Record Telemetry
            Point3D p; p.x = (float)epoch; p.y = (float)b; p.z = loss;
            loss_history_.push_back(p);

            TissNum::Matrix grad_loss = loss_function_->compute_gradient(predictions, flat_target);
            model_->backward(grad_loss);

            auto params = model_->get_parameters();
            std::vector<Parameter*> raw_params;
            for(const auto& par : params) raw_params.push_back(par.get());
            optimizer_->update(raw_params);

            if (b % 5 == 0) {
                std::cout << "\r[STELLAR] Step " << b << "/" << num_batches << " Loss: " << loss << std::flush;
            }
        }
        std::cout << std::endl;
    }

    if (visualize && !loss_history_.empty()) {
        std::cout << "\n[STELLAR] Loss Topology (3D ASCII Perspective)" << std::endl;
        std::cout << StellarVisualizer::render_3d_graph(loss_history_, 80, 25);
    }
}

} // namespace Stellar
} // namespace TissLM

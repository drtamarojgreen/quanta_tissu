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

StellarTrainer::StellarTrainer(
    std::shared_ptr<Model> model,
    std::shared_ptr<Optimizer> optimizer,
    std::shared_ptr<LossFunction> loss_fn
) : model_(model), optimizer_(optimizer), loss_function_(loss_fn) {}

void StellarTrainer::train_stellar(
    TokenDataset& dataset,
    int epochs,
    int batch_size,
    bool visualize
) {
    size_t num_samples = dataset.size();
    if (num_samples == 0) return;

    std::default_random_engine rng(1337);
    std::vector<int> indices(num_samples);
    std::iota(indices.begin(), indices.end(), 0);

    std::cout << "[STELLAR] Training Engine Activated. Implementing Core backprop..." << std::endl;

    for (int epoch = 0; epoch < epochs; ++epoch) {
        std::shuffle(indices.begin(), indices.end(), rng);
        size_t num_batches = (num_samples + batch_size - 1) / batch_size;

        for (size_t b = 0; b < num_batches; ++b) {
            size_t batch_start = b * batch_size;
            size_t batch_end = std::min((size_t)batch_start + batch_size, num_samples);
            size_t cur_size = batch_end - batch_start;

            TissNum::Matrix bin({cur_size, dataset.get_item(0).first.cols()});
            TissNum::Matrix btarg({cur_size, dataset.get_item(0).second.cols()});

            for (size_t i = 0; i < cur_size; ++i) {
                auto item = dataset.get_item(indices[batch_start + i]);
                for (size_t c = 0; c < item.first.cols(); ++c) bin({i, c}) = item.first({0, c});
                for (size_t c = 0; c < item.second.cols(); ++c) btarg({i, c}) = item.second({0, c});
            }

            // Implementation of the existing training model logic
            Matrix pred = model_->forward(bin);
            Matrix flat_target({btarg.rows() * btarg.cols(), 1});
            for (size_t r = 0; r < btarg.rows(); ++r)
                for (size_t c = 0; c < btarg.cols(); ++c)
                    flat_target({r * btarg.cols() + c, 0}) = btarg({r, c});

            float loss = loss_function_->compute_loss(pred, flat_target);

            Point3D lp; lp.x = (float)epoch; lp.y = (float)b; lp.z = loss;
            loss_history_.push_back(lp);

            Matrix grad = loss_function_->compute_gradient(pred, flat_target);
            model_->backward(grad);

            auto shared_params = model_->get_parameters();
            std::vector<Parameter*> raw_params;
            for (const auto& p : shared_params) raw_params.push_back(p.get());
            optimizer_->update(raw_params);

            if (b % 5 == 0) std::cout << "\r[STELLAR] Epoch " << epoch+1 << " [" << b << "/" << num_batches << "] Loss: " << loss << std::flush;
        }
        std::cout << std::endl;
    }

    if (visualize) {
        std::cout << "\n[STELLAR] Final Training Geometry (3D ASCII Render)" << std::endl;
        std::cout << StellarVisualizer::render_3d_graph(loss_history_, 80, 25);
    }
}

} // namespace Stellar
} // namespace TissLM

#include "trainer.h"
#include <random>
#include <chrono>
#include <algorithm> // For std::shuffle
#include <numeric>   // For std::iota
#include <fstream>

namespace TissLM {
namespace Training {

using namespace TissLM::Core;
using namespace TissNum;

Trainer::Trainer(
    std::shared_ptr<Core::Model> model,
    std::shared_ptr<Optimizer> optimizer,
    std::shared_ptr<LossFunction> loss_function
) : model_(model), optimizer_(optimizer), loss_function_(loss_function) {
}

void Trainer::train(
    TokenDataset& dataset,
    int epochs,
    int batch_size
) {
    size_t num_samples = dataset.size();
    if (num_samples == 0) {
        std::cerr << "No training data provided." << std::endl;
        return;
    }

    std::vector<int> indices(num_samples);
    std::iota(indices.begin(), indices.end(), 0);

    std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

    for (int epoch = 0; epoch < epochs; ++epoch) {
        std::shuffle(indices.begin(), indices.end(), rng);
        float epoch_loss = 0.0f;
        size_t num_batches = (num_samples + batch_size - 1) / batch_size;

        for (size_t b = 0; b < num_batches; ++b) {
            size_t batch_start = b * batch_size;
            size_t batch_end = std::min((size_t)batch_start + batch_size, num_samples);
            size_t current_batch_size = batch_end - batch_start;

            if (b % 10 == 0) {
                std::cout << "Epoch " << epoch + 1 << ", Batch " << b << "/" << num_batches << std::endl;
            }

            TissNum::Matrix batch_input({current_batch_size, dataset.get_item(0).first.cols()});
            TissNum::Matrix batch_target({current_batch_size, dataset.get_item(0).second.cols()});

            for (size_t i = 0; i < current_batch_size; ++i) {
                int sample_idx = indices[batch_start + i];
                auto item = dataset.get_item(sample_idx);
                for (size_t col = 0; col < item.first.cols(); ++col) {
                    batch_input({i, col}) = item.first({0, col});
                }
                for (size_t col = 0; col < item.second.cols(); ++col) {
                    batch_target({i, col}) = item.second({0, col});
                }
            }

            // Forward pass
            TissNum::Matrix predictions = model_->forward(batch_input);

            // Reshape target for loss computation
            TissNum::Matrix reshaped_target({batch_target.rows() * batch_target.cols(), 1});
            for (size_t r = 0; r < batch_target.rows(); ++r) {
                for (size_t c = 0; c < batch_target.cols(); ++c) {
                    reshaped_target({r * batch_target.cols() + c, 0}) = batch_target({r, c});
                }
            }

            // Compute loss
            float loss = loss_function_->compute_loss(predictions, reshaped_target);
            epoch_loss += loss;

            // Compute gradient of loss w.r.t. predictions
            TissNum::Matrix grad_loss = loss_function_->compute_gradient(predictions, reshaped_target);

            // Backward pass
            model_->backward(grad_loss);

            // Update parameters
            auto params_shared = model_->get_parameters();
            std::vector<TissNum::Parameter*> params_raw;
            params_raw.reserve(params_shared.size());
            for(const auto& p : params_shared) {
                params_raw.push_back(p.get());
            }
            optimizer_->update(params_raw);
        }
        std::cout << "Epoch " << epoch + 1 << ", Loss: " << epoch_loss / num_batches << std::endl;
    }
}

void Trainer::save_checkpoint(const std::string& path) const {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }

    // Save model parameters
    auto params = model_->get_parameters();
    size_t num_params = params.size();
    ofs.write(reinterpret_cast<const char*>(&num_params), sizeof(num_params));
    for (const auto& p : params) {
        const auto& matrix = p->value();
        auto shape = matrix.get_shape();
        size_t shape_size = shape.size();
        ofs.write(reinterpret_cast<const char*>(&shape_size), sizeof(shape_size));
        ofs.write(reinterpret_cast<const char*>(shape.data()), shape_size * sizeof(size_t));
        size_t data_size = matrix.data_size();
        ofs.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
        ofs.write(reinterpret_cast<const char*>(matrix.get_data()), data_size * sizeof(float));
    }

    // Save optimizer state
    optimizer_->save_state(ofs);
}

void Trainer::load_checkpoint(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Cannot open file for reading: " + path);
    }

    // Load model parameters
    auto params = model_->get_parameters();
    size_t num_params;
    ifs.read(reinterpret_cast<char*>(&num_params), sizeof(num_params));
    if (num_params != params.size()) {
        throw std::runtime_error("Checkpoint parameter count does not match model parameter count.");
    }
    for (const auto& p : params) {
        auto& matrix = p->value();
        size_t shape_size;
        ifs.read(reinterpret_cast<char*>(&shape_size), sizeof(shape_size));
        std::vector<size_t> shape(shape_size);
        ifs.read(reinterpret_cast<char*>(shape.data()), shape_size * sizeof(size_t));
        if (shape != matrix.get_shape()) {
            throw std::runtime_error("Checkpoint parameter shape does not match model parameter shape.");
        }
        size_t data_size;
        ifs.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        if (data_size != matrix.data_size()) {
            throw std::runtime_error("Checkpoint parameter data size does not match model parameter data size.");
        }
        ifs.read(reinterpret_cast<char*>(matrix.get_data()), data_size * sizeof(float));
    }

    // Load optimizer state
    optimizer_->load_state(ifs);
}

} // namespace Training
} // namespace TissLM
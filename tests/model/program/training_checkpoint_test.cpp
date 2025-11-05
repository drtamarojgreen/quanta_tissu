#include "../../../quanta_tissu/tisslm/program/training/trainer.h"
#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/training/dataset.h"
#include "../../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../../quanta_tissu/tisslm/program/training/loss_function.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>

void check(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[  PASSED  ] " << test_name << std::endl;
    } else {
        std::cout << "[  FAILED  ] " << test_name << std::endl;
        throw std::runtime_error("Test failed: " + test_name);
    }
}

void test_save_and_load() {
    std::cout << "--- Testing Save and Load ---" << std::endl;
    auto model = std::make_shared<TissLM::Core::TransformerModel>(10, 100, 16, 4, 2, 0.1f, 4);
    auto optimizer = std::make_shared<TissLM::Training::Adam>();
    auto loss_fn = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    TissLM::Training::Trainer trainer(model, optimizer, loss_fn);

    trainer.save_checkpoint("test_checkpoint.bin");

    auto new_model = std::make_shared<TissLM::Core::TransformerModel>(10, 100, 16, 4, 2, 0.1f, 4);
    auto new_optimizer = std::make_shared<TissLM::Training::Adam>();
    TissLM::Training::Trainer new_trainer(new_model, new_optimizer, loss_fn);
    new_trainer.load_checkpoint("test_checkpoint.bin");

    auto params1 = model->get_parameters();
    auto params2 = new_model->get_parameters();
    check(params1.size() == params2.size(), "Parameter count match");
    for (size_t i = 0; i < params1.size(); ++i) {
        TissNum::Matrix p1 = params1[i]->value();
        TissNum::Matrix p2 = params2[i]->value();
        check(p1.get_shape() == p2.get_shape(), "Parameter shape match");
        bool values_match = true;
        const float* p1_data = p1.get_data();
        const float* p2_data = p2.get_data();
        for (size_t j = 0; j < p1.data_size(); ++j) {
            if (p1_data[j] != p2_data[j]) {
                values_match = false;
                break;
            }
        }
        check(values_match, "Parameter value match for param " + std::to_string(i));
    }
}

int main() {
    try {
        test_save_and_load();
        std::cout << "\nAll Training checkpoint tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTraining checkpoint tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
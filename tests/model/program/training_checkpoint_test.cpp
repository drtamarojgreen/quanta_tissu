#include "gtest/gtest.h"
#include "training/trainer.h"
#include "core/transformer_model.h"
#include "training/dataset.h"

TEST(TrainingCheckpointTest, SaveAndLoad) {
    TissLM::Core::TransformerModel model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::Trainer trainer(&model, nullptr);

    trainer.save_checkpoint("test_checkpoint.bin");

    TissLM::Core::TransformerModel new_model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::Trainer new_trainer(&new_model, nullptr);
    new_trainer.load_checkpoint("test_checkpoint.bin");

    auto params1 = model.parameters();
    auto params2 = new_model.parameters();
    for (size_t i = 0; i < params1.size(); ++i) {
        TissNum::Matrix p1 = params1[i]->value();
        TissNum::Matrix p2 = params2[i]->value();
        EXPECT_EQ(p1.get_shape(), p2.get_shape());
        for (size_t r = 0; r < p1.rows(); ++r) {
            for (size_t c = 0; c < p1.cols(); ++c) {
                EXPECT_EQ(p1({r, c}), p2({r, c}));
            }
        }
    }
}

TEST(TrainingCheckpointTest, ContinueTraining) {
    // Create a model and a trainer
    TissLM::Core::TransformerModel model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::AdamOptimizer optimizer(model.parameters(), 1e-4);
    TissLM::Training::Trainer trainer(&model, &optimizer);

    // Create a dummy dataset
    TissNum::Matrix data = TissNum::Matrix::random({10, 10});
    TissLM::Training::Dataset dataset(data);

    // Train for a few steps
    trainer.train(dataset, 2, 1, 1e-4);
    float loss1 = trainer.get_last_loss();

    // Save a checkpoint
    trainer.save_checkpoint("test_checkpoint.bin");

    // Create a new model and trainer
    TissLM::Core::TransformerModel new_model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::AdamOptimizer new_optimizer(new_model.parameters(), 1e-4);
    TissLM::Training::Trainer new_trainer(&new_model, &new_optimizer);

    // Load the checkpoint
    new_trainer.load_checkpoint("test_checkpoint.bin");

    // Continue training
    new_trainer.train(dataset, 2, 1, 1e-4);
    float loss2 = new_trainer.get_last_loss();

    EXPECT_LT(loss2, loss1);
}

TEST(TrainingCheckpointTest, InvalidPath) {
    TissLM::Core::TransformerModel model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::Trainer trainer(&model, nullptr);

    EXPECT_THROW(trainer.save_checkpoint("/invalid/path/checkpoint.bin"), std::runtime_error);
    EXPECT_THROW(trainer.load_checkpoint("/invalid/path/checkpoint.bin"), std::runtime_error);
}

TEST(TrainingCheckpointTest, OptimizerState) {
    TissLM::Core::TransformerModel model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::AdamOptimizer optimizer(model.parameters(), 1e-4);
    TissLM::Training::Trainer trainer(&model, &optimizer);

    trainer.save_checkpoint("test_checkpoint.bin");

    TissLM::Core::TransformerModel new_model(10, 4, 2, 16, 100, 0.1, 4);
    TissLM::Training::AdamOptimizer new_optimizer(new_model.parameters(), 1e-4);
    TissLM::Training::Trainer new_trainer(&new_model, &new_optimizer);

    new_trainer.load_checkpoint("test_checkpoint.bin");

    // Check that the optimizer state is the same
    EXPECT_EQ(optimizer.get_step(), new_optimizer.get_step());
}
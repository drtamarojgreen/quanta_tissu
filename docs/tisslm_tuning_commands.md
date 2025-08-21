# TissLM Model Tuning Commands

This document provides a set of example commands for training and tuning the TissLM model from the project root directory.

The main training script is `quanta_tissu/tisslm/core/run_training.py`.

## Basic Training

This command starts a new training run. It uses the text files in the `corpus/` directory as training data and saves model checkpoints to the `checkpoints/` directory.

```bash
python3 -m quanta_tissu.tisslm.core.run_training \
    --corpus_path corpus/ \
    --checkpoint_dir checkpoints/ \
    --save_every 200
```

- `--corpus_path`: Specifies the directory containing the training data.
- `--checkpoint_dir`: Specifies the directory where model checkpoints will be saved.
- `--save_every`: Sets the frequency (in training steps) for saving a new checkpoint.

## Resuming Training

This command resumes a previous training run from a specific checkpoint. This is useful if your training was interrupted or if you want to continue training a model.

```bash
python3 -m quanta_tissu.tisslm.core.run_training \
    --corpus_path corpus/ \
    --checkpoint_dir checkpoints/ \
    --resume_from checkpoints/checkpoint_step_200.npz
```

- `--resume_from`: Specifies the path to the checkpoint file to load and resume from.

## Advanced Tuning

This command shows how to tune various hyperparameters. In this example, we are also saving the final models to a `models/` directory, as it can be useful to distinguish final models from periodic checkpoints.

```bash
python3 -m quanta_tissu.tisslm.core.run_training \
    --corpus_path corpus/ \
    --checkpoint_dir models/ \
    --epochs 10 \
    --batch_size 16 \
    --lr 0.0001 \
    --warmup_steps 100 \
    --save_every 500
```

- `--epochs`: Number of times to iterate over the entire dataset.
- `--batch_size`: Number of training examples to use in a single step.
- `--lr`: The learning rate for the optimizer.
- `--warmup_steps`: Number of initial steps during which the learning rate gradually increases.

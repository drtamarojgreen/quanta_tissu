# TissLM C++ Training Module

This directory contains the C++ implementation of training capabilities for the TissLM language model.

## Overview

The training module provides a complete training loop with gradient computation, optimization, and checkpoint management.

## Components

### Trainer Class

The main trainer class encapsulates the entire training process.

**Core Responsibilities:**
- Training loop management
- Forward and backward passes
- Gradient computation and clipping
- Optimizer integration
- Learning rate scheduling
- Checkpoint saving/loading
- Training metrics tracking

### Training Configuration

**Key Parameters:**
- `epochs`: Number of training epochs
- `batch_size`: Batch size for training
- `learning_rate`: Initial learning rate
- `max_grad_norm`: Gradient clipping threshold
- `save_every`: Checkpoint frequency (steps)
- `checkpoint_dir`: Directory for checkpoints
- `keep_checkpoints`: Number of checkpoints to retain
- `resume_from`: Path to resume training from

### Backward Pass Implementation

The trainer implements backward passes for all model components:

**Layer-wise Backpropagation:**
1. **Output Layer**: Compute gradients for final projection
2. **Transformer Blocks**: Backpropagate through each block
3. **Layer Normalization**: Compute gradients for gamma, beta, and input
4. **Feed-Forward Network**: Backpropagate through FFN layers
5. **Multi-Head Attention**: Compute gradients for Q, K, V, and output projections
6. **Embeddings**: Accumulate gradients for token embeddings

**Gradient Clipping:**
- Computes global gradient norm
- Clips gradients if norm exceeds threshold
- Prevents exploding gradients

## Planned Implementation

### Trainer Class Structure

```cpp
class Trainer {
public:
    Trainer(Model* model, 
            Optimizer* optimizer,
            LossFunction* loss_fn,
            LRScheduler* scheduler,
            Dataset* dataset,
            const TrainingConfig& config);
    
    // Main training loop
    void train();
    
    // Single training step
    float train_step(const Matrix& x_batch, const Matrix& y_batch);
    
    // Validation
    float validate(Dataset* val_dataset);
    
    // Checkpoint management
    void save_checkpoint(int epoch, int step);
    void load_checkpoint(const std::string& path);
    
private:
    // Backward pass implementations
    void backward(const Matrix& d_logits, const ModelCache& cache);
    void backward_transformer_block(Matrix& d_out, const BlockCache& cache);
    void backward_layernorm(Matrix& d_out, const LayerNormCache& cache);
    void backward_ffn(Matrix& d_out, const FFNCache& cache);
    void backward_mha(Matrix& d_out, const MHACache& cache);
    
    // Gradient utilities
    float compute_grad_norm();
    void clip_gradients(float max_norm);
    void zero_gradients();
    
    Model* model_;
    Optimizer* optimizer_;
    LossFunction* loss_fn_;
    LRScheduler* scheduler_;
    Dataset* dataset_;
    TrainingConfig config_;
    
    // Training state
    int current_epoch_;
    int current_step_;
    std::vector<float> loss_history_;
};
```

### Training Configuration

```cpp
struct TrainingConfig {
    int epochs = 10;
    int batch_size = 32;
    float learning_rate = 1e-4f;
    float max_grad_norm = 1.0f;
    int save_every = 1000;
    std::string checkpoint_dir = "checkpoints";
    int keep_checkpoints = 3;
    std::string resume_from = "";
    bool use_mixed_precision = false;
    int gradient_accumulation_steps = 1;
    bool log_gradients = false;
    int validation_frequency = 100;
};
```

### Optimizer Integration

**Supported Optimizers:**
- **SGD**: Stochastic Gradient Descent
- **Adam**: Adaptive Moment Estimation
- **AdamW**: Adam with weight decay
- **RMSprop**: Root Mean Square Propagation

**Optimizer Interface:**
```cpp
class Optimizer {
public:
    virtual void step() = 0;
    virtual void zero_grad() = 0;
    virtual void set_lr(float lr) = 0;
    virtual float get_lr() const = 0;
};
```

### Learning Rate Scheduling

**Scheduler Types:**
- **Constant**: Fixed learning rate
- **Step Decay**: Reduce LR at fixed intervals
- **Exponential Decay**: Exponential reduction
- **Cosine Annealing**: Cosine schedule
- **Warmup + Decay**: Linear warmup followed by decay

**Scheduler Interface:**
```cpp
class LRScheduler {
public:
    virtual float step(int global_step) = 0;
    virtual float get_lr() const = 0;
};
```

### Loss Functions

**Implemented Losses:**
- **Cross-Entropy**: Standard language modeling loss
- **Label Smoothing**: Regularized cross-entropy
- **Focal Loss**: Focus on hard examples
- **Contrastive Loss**: For embedding learning

**Loss Interface:**
```cpp
class LossFunction {
public:
    virtual float forward(const Matrix& logits, const Matrix& targets) = 0;
    virtual Matrix backward() = 0;
};
```

## Implementation Status

### Completed
✅ Training architecture design
✅ Backward pass algorithms documented
✅ Optimizer interfaces defined
✅ Checkpoint strategy planned

### In Progress
🔄 Trainer class implementation
🔄 Optimizer implementations (Adam, SGD)
🔄 Learning rate schedulers
🔄 Loss function implementations

### Planned
📋 Mixed precision training
📋 Distributed training support
📋 Gradient accumulation
📋 Training visualization
📋 Early stopping
📋 Model evaluation metrics

## Usage Examples

### Basic Training

```cpp
#include "trainer.h"
#include "optimizers/adam.h"
#include "schedulers/cosine.h"
#include "losses/cross_entropy.h"

// Create model
Model model(config);

// Create optimizer
AdamOptimizer optimizer(model.parameters(), 1e-4f);

// Create scheduler
CosineScheduler scheduler(optimizer, 10000, 1000);

// Create loss function
CrossEntropyLoss loss_fn;

// Create dataset
Dataset dataset("train.txt", tokenizer);

// Configure training
TrainingConfig train_config;
train_config.epochs = 10;
train_config.batch_size = 32;
train_config.max_grad_norm = 1.0f;
train_config.save_every = 1000;
train_config.checkpoint_dir = "checkpoints";

// Create trainer
Trainer trainer(&model, &optimizer, &loss_fn, &scheduler, &dataset, train_config);

// Train
trainer.train();
```

### Resume Training

```cpp
TrainingConfig config;
config.resume_from = "checkpoints/checkpoint_step_5000.bin";

Trainer trainer(&model, &optimizer, &loss_fn, &scheduler, &dataset, config);
trainer.train();
```

### Custom Training Loop

```cpp
for (int epoch = 0; epoch < num_epochs; ++epoch) {
    for (auto [x_batch, y_batch] : dataset) {
        // Forward pass
        auto [logits, cache] = model.forward(x_batch);
        
        // Compute loss
        float loss = loss_fn.forward(logits, y_batch);
        
        // Backward pass
        auto d_logits = loss_fn.backward();
        trainer.backward(d_logits, cache);
        
        // Clip gradients
        trainer.clip_gradients(1.0f);
        
        // Update parameters
        optimizer.step();
        
        // Update learning rate
        float lr = scheduler.step(step);
        
        // Zero gradients
        optimizer.zero_grad();
        
        // Log
        if (step % 10 == 0) {
            std::cout << "Step " << step << ", Loss: " << loss 
                     << ", LR: " << lr << std::endl;
        }
        
        // Save checkpoint
        if (step % 1000 == 0) {
            trainer.save_checkpoint(epoch, step);
        }
        
        step++;
    }
}
```

## Gradient Computation Details

### Layer Normalization Backward

```cpp
// Compute gradients for gamma and beta
beta.grad += d_out.sum(axis=(0, 1));
gamma.grad += (d_out * x_norm).sum(axis=(0, 1));

// Gradient for normalized input
dx_norm = d_out * gamma;

// Gradient for variance
dvar = sum(dx_norm * (x - mean) * -0.5 * (var + eps)^(-1.5));

// Gradient for mean
dmean = sum(dx_norm * -1 / sqrt(var + eps)) - 2 * dvar * sum(x - mean) / D;

// Final gradient for input
dx = (dx_norm / sqrt(var + eps)) + (dvar * 2 * (x - mean) / D) + (dmean / D);
```

### Multi-Head Attention Backward

```cpp
// Output projection gradient
Wo.grad += combined^T @ d_out;
d_combined = d_out @ Wo^T;

// Split heads
d_attended = split_heads(d_combined);

// Attention weights gradient
d_weights = d_attended @ V^T;
dV = attention_weights^T @ d_attended;

// Scores gradient (through softmax)
d_scores = d_weights * sqrt(d_k);

// Q, K gradients
dQ = d_scores @ K;
dK = Q^T @ d_scores;

// Projection gradients
Wq.grad += x^T @ dQ;
Wk.grad += x^T @ dK;
Wv.grad += x^T @ dV;

// Input gradient
dx = (dQ @ Wq^T) + (dK @ Wk^T) + (dV @ Wv^T);
```

## Performance Optimization

**Memory Efficiency:**
- Gradient checkpointing for large models
- In-place operations where possible
- Efficient cache management

**Computation Efficiency:**
- Vectorized operations
- Parallel batch processing
- Optimized BLAS operations

**Training Speed:**
- Mixed precision training (FP16)
- Gradient accumulation for large batches
- Distributed data parallel training

## Monitoring and Debugging

**Training Metrics:**
- Loss per step
- Gradient norms
- Learning rate
- Throughput (tokens/sec)
- Memory usage

**Debugging Tools:**
- Gradient checking
- Activation statistics
- Weight distribution tracking
- NaN/Inf detection

## Integration

The training module integrates with:
- **Core Model**: All transformer components
- **Optimizers**: Parameter updates
- **Data Loading**: Batch generation
- **Checkpointing**: Model persistence
- **Evaluation**: Validation metrics

## References

- [Adam: A Method for Stochastic Optimization](https://arxiv.org/abs/1412.6980)
- [Decoupled Weight Decay Regularization](https://arxiv.org/abs/1711.05101) - AdamW
- [Mixed Precision Training](https://arxiv.org/abs/1710.03740)
- [Gradient Checkpointing](https://arxiv.org/abs/1604.06174)

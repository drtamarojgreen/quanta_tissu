# TissLM C++ Generation Module

This directory contains the C++ implementation of text generation capabilities for the TissLM language model.

## Overview

The generation module provides various sampling strategies and decoding methods for generating text from the trained language model.

## Components

### GenerationConfig (`generation_config.h`)

Configuration structure for controlling text generation behavior.

**Sampling Methods:**
- **Greedy**: Always select the most probable token
- **Top-k**: Sample from the k most probable tokens
- **Nucleus (Top-p)**: Sample from tokens with cumulative probability ≥ p
- **Top-a**: Sample from tokens with probability > a
- **Random**: Sample from all tokens according to their probabilities
- **Beam Search**: Maintain multiple hypotheses and select the best
- **Contrastive Search**: Balance model confidence with diversity
- **Mirostat**: Adaptive sampling based on perplexity control

**Parameters:**
- `temperature`: Controls randomness (higher = more random)
- `repetition_penalty`: Discourages token repetition
- `no_repeat_ngram_size`: Prevents repeating n-grams
- `logit_bias`: Apply bias to specific tokens
- `eos_ids`: End-of-sequence token IDs
- `beam_width`: Number of beams for beam search
- `contrastive_alpha`: Balance parameter for contrastive search
- `mirostat_tau`, `mirostat_eta`: Mirostat sampling parameters

**Factory Methods:**
```cpp
// Greedy decoding
auto config = GenerationConfig::greedy();

// Random sampling with temperature
auto config = GenerationConfig::sampling(0.8f);

// Top-k sampling
auto config = GenerationConfig::top_k(50, 0.9f);

// Nucleus sampling
auto config = GenerationConfig::nucleus(0.95f, 0.8f);
```

## Planned Components

### Generator Class

The main generator class will provide:

**Core Methods:**
- `sample()`: Generate tokens using configured sampling strategy
- `beam_search()`: Generate using beam search
- `contrastive_search()`: Generate using contrastive decoding
- `mirostat_sampling()`: Generate using Mirostat
- `speculative_sampling()`: Fast generation with draft model

**Advanced Features:**
- `sample_with_context()`: RAG-style generation with document context
- `calculate_source_attribution()`: Track which sources influenced generation
- Attention weight tracking for interpretability
- Batch generation support

### Sampling Strategies

**Greedy Sampling:**
- Deterministic
- Always selects argmax
- Fast but can be repetitive

**Top-k Sampling:**
- Restricts to k most probable tokens
- Balances quality and diversity
- Common choice: k=50

**Nucleus (Top-p) Sampling:**
- Dynamic vocabulary based on cumulative probability
- More adaptive than top-k
- Common choice: p=0.95

**Top-a Sampling:**
- Absolute probability threshold
- Filters low-probability tokens
- Good for controlling quality

**Contrastive Search:**
- Balances model confidence and diversity
- Penalizes similarity to context
- Reduces repetition effectively

**Mirostat Sampling:**
- Controls perplexity dynamically
- Adapts to maintain target surprise level
- Good for long-form generation

## Implementation Status

### Completed
✅ GenerationConfig structure with all parameters
✅ Factory methods for common configurations
✅ Documentation of sampling strategies

### In Progress
🔄 Generator class implementation
🔄 Sampling strategy implementations
🔄 Attention weight tracking

### Planned
📋 Speculative decoding
📋 Constrained generation
📋 Guided generation with classifiers
📋 Multi-modal generation support

## Usage Examples

### Basic Generation

```cpp
#include "generation_config.h"
#include "generator.h"

// Create generator with model
Generator generator(model, tokenizer);

// Configure generation
auto config = GenerationConfig::nucleus(0.95f, 0.8f);
config.repetition_penalty = 1.2f;
config.no_repeat_ngram_size = 3;
config.eos_ids = {tokenizer.eos_id()};

// Generate text
std::vector<int> prompt_tokens = tokenizer.encode("Once upon a time");
auto generated = generator.sample(prompt_tokens, 100, config);

// Decode
std::string text = tokenizer.decode(generated);
```

### Beam Search

```cpp
auto config = GenerationConfig::greedy();
config.method = "beam_search";
config.beam_width = 5;
config.eos_ids = {tokenizer.eos_id()};

auto generated = generator.beam_search(prompt_tokens, 100, config);
```

### RAG Generation

```cpp
std::vector<std::string> documents = {
    "Document 1 content...",
    "Document 2 content...",
    "Document 3 content..."
};

auto config = GenerationConfig::nucleus(0.9f);
auto [generated, attention, spans] = generator.sample_with_context(
    "What is the answer?",
    documents,
    100,
    config
);

// Calculate source attribution
auto attribution = generator.calculate_source_attribution(attention, spans);
```

## Integration

The generation module integrates with:
- **Core Model**: Uses transformer model for forward passes
- **Tokenizer**: Encodes/decodes text
- **RAG Pipeline**: Provides context-aware generation
- **Training**: Generates samples during training for evaluation

## Performance Considerations

**Optimization Strategies:**
- KV-cache for efficient autoregressive generation
- Batch generation for throughput
- Speculative decoding for speed
- Quantization for memory efficiency

**Memory Management:**
- Reuse buffers across generation steps
- Stream generation for long sequences
- Efficient attention computation

## Testing

Test coverage will include:
- Sampling strategy correctness
- Temperature effects
- Repetition penalty behavior
- Beam search quality
- Edge cases (empty prompts, long sequences)
- Performance benchmarks

## References

- [The Curious Case of Neural Text Degeneration](https://arxiv.org/abs/1904.09751) - Nucleus sampling
- [Contrastive Search](https://arxiv.org/abs/2210.14140) - Contrastive decoding
- [Mirostat](https://arxiv.org/abs/2007.14966) - Perplexity-controlled sampling
- [Speculative Decoding](https://arxiv.org/abs/2211.17192) - Fast generation

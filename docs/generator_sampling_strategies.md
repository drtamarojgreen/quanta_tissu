# Integrating Retrieval Strategies into Algorithmic Generator Sampling

This document outlines a plan to create a new module, `alg_generator.py`, which will extend the functionality of the existing `generator.py` by integrating `CNNSimilarityStrategy` and `BayesianSimilarityStrategy` into its token sampling process. The goal is to allow these strategies to influence token selection while preserving existing functionality and ensuring minimal impact on the current application.

## 1. Understanding the Challenge

The `CNNSimilarityStrategy` and `BayesianSimilarityStrategy` are designed for calculating similarity between *embeddings*, not directly for sampling tokens from a language model's logits. The existing `Generator` class's `_predict_from_logits` method currently operates on raw logits (or probabilities derived from them) to select the next token based on methods like greedy, top-k, nucleus, and random sampling.

To integrate these new strategies, we need a mechanism to:
*   Obtain relevant embeddings during the token generation process.
*   Use these embeddings with the similarity strategies to influence the probability distribution of the next token.

## 2. Proposed Implementation: Creating `alg_generator.py`

Instead of modifying the existing `generator.py` directly, we will create a new module `alg_generator.py` that will house the extended functionality. This ensures that the original `generator.py` remains untouched, preserving existing application functionality.

### 2.1. Creation of `alg_generator.py`

`alg_generator.py` will be created as a copy of `quanta_tissu/tisslm/core/generation/generator.py`.

### 2.2. Modifications to `alg_generator.py`

The integration will primarily occur within the `Generator` class (now in `alg_generator.py`), specifically by extending the `_predict_from_logits` method and potentially introducing new helper methods.

**a. Initialization (`__init__`)**:
The `Generator` class in `alg_generator.py` will need access to:
*   The model's embedding layer (to get token embeddings).
*   Instances of `CNNSimilarityStrategy` and `BayesianSimilarityStrategy`.
*   Potentially, a mechanism to provide a "query embedding" if these strategies are to be used in a retrieval-augmented manner during generation.

```python
# In quanta_tissu/tisslm/core/generation/alg_generator.py

import numpy as np
import logging

from ..layers import softmax
from ..model_error_handler import ConfigurationError, ModelProcessingError
from ..retrieval.strategy import CNNSimilarityStrategy, BayesianSimilarityStrategy

logger = logging.getLogger(__name__)

class Generator:
    def __init__(self, model):
        self.model = model
        # Initialize similarity strategies
        embedding_dim = model.config.get('embedding_dim', 128) # Example: get from model config
        self.cnn_strategy = CNNSimilarityStrategy(embedding_dim=embedding_dim)
        self.bayesian_strategy = BayesianSimilarityStrategy()
        # Note: BayesianStrategy might need a way to get 'hessian_matrix'
        # This will require further design on how to provide this context during generation.
```

**b. Extending `_predict_from_logits`**:
This method is the core of token selection. We will introduce new `method` types (e.g., "cnn_influenced", "bayesian_influenced") that leverage the similarity strategies.

```python
# In quanta_tissu/tisslm/core/generation/alg_generator.py

class Generator:
    # ... existing methods ...

    def _predict_from_logits(self, logits, method="greedy", temperature=1.0, top_k=None, top_p=None, past_tokens=None, repetition_penalty=1.0, eos_id=None, bias_token_id=None, bias_strength=0.0, **kwargs):
        # ... existing logic for repetition penalty, telemetry, softmax ...

        # --- New Integration for CNN/Bayesian Influenced Sampling ---
        if method == "cnn_influenced":
            query_embedding = kwargs.get('query_embedding')
            if query_embedding is None:
                raise ValueError("cnn_influenced method requires 'query_embedding' in kwargs.")

            all_vocab_embeddings = self.model.embeddings.value # Shape: (vocab_size, embedding_dim)
            
            similarity_scores = self.cnn_strategy.calculate_similarity(query_embedding, all_vocab_embeddings)
            
            scaled_similarity = similarity_scores * kwargs.get('cnn_influence_scale', 1.0)
            
            logits = logits + scaled_similarity
            probs = softmax(logits, temperature=temperature) # Re-calculate probabilities
            next_token = int(np.argmax(probs)) # Default to greedy after influence

        elif method == "bayesian_influenced":
            query_embedding = kwargs.get('query_embedding')
            hessian_matrix = kwargs.get('hessian_matrix')
            if query_embedding is None or hessian_matrix is None:
                raise ValueError("bayesian_influenced method requires 'query_embedding' and 'hessian_matrix' in kwargs.")

            all_vocab_embeddings = self.model.embeddings.value # Shape: (vocab_size, embedding_dim)
            
            similarity_scores = self.bayesian_strategy.calculate_similarity(query_embedding, all_vocab_embeddings, hessian_matrix=hessian_matrix)
            
            scaled_similarity = similarity_scores * kwargs.get('bayesian_influence_scale', 1.0)
            
            logits = logits + scaled_similarity
            probs = softmax(logits, temperature=temperature) # Re-calculate probabilities
            next_token = int(np.argmax(probs)) # Default to greedy after influence

        else:
            # Fallback to existing methods if new ones are not selected
            # This ensures existing functionality is preserved.
            # ... existing logic for greedy, top-k, nucleus, random sampling ...
            # (Copy the relevant parts from original generator.py here)
            
            # For brevity in this plan, assume existing logic is here.
            # In actual implementation, copy the full if/elif chain for greedy, top_k, nucleus, random.
            
            # Example of how existing logic would be structured:
            if method == "greedy":
                next_token = int(np.argmax(probs))
            elif method == "top_k":
                # ... top_k logic ...
                pass
            elif method == "nucleus":
                # ... nucleus logic ...
                pass
            elif method == "random" or method == "sampling":
                # ... random logic ...
                pass
            else:
                raise ConfigurationError(f"Unknown sampling method: {method}")
        
        return next_token, probs
```

### 2.3. Updating Usage of Generator

To use the new `alg_generator`, any code that currently imports and instantiates `Generator` from `quanta_tissu/tisslm/core/generation/generator.py` will need to be updated to import from `quanta_tissu/tisslm/core/generation/alg_generator.py` instead.

For example, in `quanta_tissu/tisslm/core/model.py`:

```python
# In quanta_tissu/tisslm/core/model.py

# Change this:
# from .generation.generator import Generator

# To this:
from .generation.alg_generator import Generator as AlgorithmicGenerator

class QuantaTissu:
    def __init__(self, config, db_host='127.0.0.1', db_port=8080, use_db=False):
        self.config = config
        self.model = Model(config)
        # Change this:
        # self.generator = Generator(self.model)
        # To this:
        self.generator = AlgorithmicGenerator(self.model)
        # ... rest of __init__ ...
```

## 3. Considerations and Dependencies

*   **Embedding Access:** The `Generator` class needs a reliable way to get embeddings for all tokens in the vocabulary (`self.model.embeddings.value`). This assumes `model.embeddings` is an accessible property that returns the embedding matrix.
*   **Query Embedding:** How will the `query_embedding` be determined for each token generation step?
    *   **Fixed Query:** A single query embedding provided at the start of generation (e.g., representing a topic).
    *   **Dynamic Query:** The embedding of the last generated token, or a contextual embedding derived from `past_tokens`. This is more complex as `_predict_from_logits` doesn't have direct access to the `tokenizer` to get embeddings for `past_tokens`. This might require passing the `tokenizer` to the `Generator` or having the `Model` expose a method to get embeddings from token IDs.
*   **Hessian Matrix for Bayesian Strategy:** The `BayesianSimilarityStrategy` requires a `hessian_matrix`. This is typically derived from a training process and represents uncertainty. How will this be provided during inference? It might need to be loaded as part of the model or passed as a `kwargs` argument.
*   **Influence Scaling:** The `cnn_influence_scale` and `bayesian_influence_scale` parameters are crucial for controlling how much the similarity scores influence the token probabilities. These will need to be tuned.
*   **Performance:** Calculating similarity scores against the entire vocabulary at each step can be computationally expensive, especially for large vocabularies. Optimization might be needed (e.g., applying strategies only to top-N candidates from initial logits).
*   **Existing Sampling Methods:** The new methods (`cnn_influenced`, `bayesian_influenced`) will be new options for the `method` parameter. The existing `greedy`, `top_k`, `nucleus`, and `random` methods will remain unchanged.

## 4. Testing Strategy

*   **Unit Tests:** Add unit tests for the `Generator` class to verify that the new methods correctly influence token probabilities based on provided embeddings and similarity scores. Mock the `model.embeddings` and `similarity_strategy.calculate_similarity` calls.
*   **Integration Tests:** Run `generate_algorithmic.py` with the new methods and observe the generated text. Compare with baseline generations to understand the qualitative impact of the influence.
*   **Performance Benchmarking:** Measure the generation speed with and without these new strategies to identify any performance bottlenecks.

## 5. Future Enhancements

*   **Dynamic Query Generation:** Implement more sophisticated ways to derive `query_embedding` from the current generation context.
*   **Adaptive Influence:** Make the influence scale dynamic based on confidence or other metrics.
*   **Hybrid Sampling:** Combine these influenced methods with existing sampling (e.g., apply CNN influence, then perform top-k sampling).
*   **Pre-computation:** If `all_vocab_embeddings` are static, pre-compute and store them for faster access.
# QuantaTissu

QuantaTissu is a minimal transformer-based language model inference engine built **from scratch** in Python using only NumPy.  
It implements basic tokenization, embeddings, positional encoding, multi-head self-attention, feed-forward layers, and output projection.

## Features

- Tiny toy vocabulary tokenizer
- Positional encoding (sinusoidal)
- Single transformer decoder block
- Greedy next-token prediction

## Requirements

- Python 3.7+
- NumPy

## Usage

```bash
python quanta_tissu.py
```

## Code Overview

The `quanta_tissu.py` file contains all the components of the model:

-   **`tokenize(text)` / `detokenize(token_ids)`**: Functions to convert text to a sequence of token IDs and back.
-   **`softmax(x)`**: The softmax activation function.
-   **`LayerNorm`**: A layer normalization class.
-   **`scaled_dot_product_attention(Q, K, V)`**: The core attention mechanism.
-   **`MultiHeadAttention`**: A multi-head self-attention layer.
-   **`FeedForward`**: A position-wise feed-forward network.
-   **`TransformerBlock`**: A single block of the transformer, combining multi-head attention and a feed-forward network.
-   **`PositionalEncoding`**: A class to add positional information to the input embeddings.
-   **`QuantaTissu`**: The main class that ties everything together. It includes the embedding layer, the transformer block, and the final output projection.
-   **`if __name__ == "__main__"`**: An example of how to use the model to predict the next token for a given prompt.

## How it Works

The model takes a sequence of tokens as input and predicts the next token in the sequence. Here's a step-by-step breakdown of the process:

1.  **Tokenization**: The input text is converted into a sequence of token IDs using a simple vocabulary.
2.  **Embeddings**: Each token ID is mapped to a dense vector representation called an embedding.
3.  **Positional Encoding**: Sinusoidal positional encodings are added to the embeddings to give the model information about the position of each token in the sequence.
4.  **Transformer Block**: The sequence of embeddings is processed by a transformer block, which consists of:
    *   A multi-head self-attention layer, which allows the model to weigh the importance of different tokens in the input sequence when processing each token.
    *   A layer normalization step.
    *   A feed-forward neural network.
    *   Another layer normalization step.
5.  **Output Projection**: The output of the transformer block is passed through a final linear layer (output projection) to produce a vector of logits for each token in the input sequence.
6.  **Prediction**: The model uses a greedy approach to predict the next token. It selects the token with the highest logit value from the last position in the output sequence.

## Limitations

This is a toy model and is not suitable for any real-world applications. It has several limitations:

-   **No Training**: The model weights are randomly initialized and are not trained on any data. This means the model's predictions are essentially random.
-   **Tiny Vocabulary**: The vocabulary is extremely small and can only handle a few predefined words.
-   **Greedy Decoding**: The model uses a simple greedy decoding strategy, which may not produce the most optimal sequence of tokens.
-   **Single Transformer Block**: The model has only a single transformer block, which limits its ability to learn complex patterns.

## Future Work

There are many ways this project could be extended. Here are a few ideas:

-   **Implement a Training Pipeline**: Add a training loop, a loss function (e.g., cross-entropy), and an optimizer (e.g., Adam) to train the model on a real dataset.
-   **Expand the Vocabulary**: Use a more sophisticated tokenizer (e.g., SentencePiece) and a larger vocabulary.
-   **Add More Transformer Blocks**: Stack multiple transformer blocks to create a deeper model.
-   **Implement Beam Search**: Replace the greedy decoding with a more advanced decoding strategy like beam search to improve the quality of the generated text.
-   **Add a Causal Attention Mask**: Implement a causal attention mask to prevent the model from attending to future tokens during training.

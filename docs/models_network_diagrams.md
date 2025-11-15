# TISSLM Model Network Diagrams

This document contains highly detailed ASCII art network state diagrams for the C++,
Python, and JavaScript TISSLM models, showing class-level components and
operational descriptions.

## C++ Model (`quanta_tissu/tisslm/program`)

```
+-----------------------------------------------------------------------------+
| QuantaTissu (quantatissu.h)                                                 |
|-----------------------------------------------------------------------------|
| - model: Model                                                              |
| - tokenizer: Tokenizer                                                      |
|-----------------------------------------------------------------------------|
| + generate(prompt, n_tokens): string                                        |
+-----------------------------------------------------------------------------+
       |
       | owns
       v
+-----------------------------------------------------------------------------+
| Tokenizer (tokenizer/tokenizer.h)                                           |
|-----------------------------------------------------------------------------|
| - pre_tokenizer: PreTokenizer                                               |
| - vocab: map<string, int>                                                   |
|-----------------------------------------------------------------------------|
| + encode(text): vector<int>                                                 |
| + decode(ids): string                                                       |
+-----------------------------------------------------------------------------+
       |
       | uses
       v
+-----------------------------------------------------------------------------+
| Model (architecture/model.h)                                                |
|-----------------------------------------------------------------------------|
| - embedding: Embedding                                                      |
| - pos_encoding: PositionalEncoding                                          |
| - transformer_blocks: vector<TransformerBlock>                              |
| - layer_norm: LayerNorm                                                     |
|-----------------------------------------------------------------------------|
| + forward(input_ids): Matrix                                                |
+-----------------------------------------------------------------------------+
       |
       | owns/composes
       |
       +----------------> TransformerBlock (core/transformerblock.h)
       |                     |
       |                     +----> MultiHeadAttention (core/multiheadattention.h)
       |                     |
       |                     +----> FeedForward (core/feedforward.h)
       |                     |
       |                     +----> LayerNorm (core/layernorm.h)
       |
       +----------------> Embedding (core/embedding.h)
       |
       +----------------> PositionalEncoding (core/positionalencoding.h)
       |
       | uses
       v
+-----------------------------------------------------------------------------+
| Generator (generation/generator.h)                                          |
|-----------------------------------------------------------------------------|
| - config: GenerationConfig                                                  |
|-----------------------------------------------------------------------------|
| + generate(model, input_ids, n_tokens): vector<int>                         |
+-----------------------------------------------------------------------------+
```

### Core Architecture Operations

The C++ TISSLM architecture is orchestrated by the `QuantaTissu` class, which encapsulates the core functionality. On a call to `generate`, the input prompt is first processed by the `Tokenizer`. This component, which may use a `PreTokenizer`, converts the text into a sequence of integer token IDs based on its vocabulary.

These token IDs are then passed to the `Model` object. The model begins by converting the token IDs into dense vectors using an `Embedding` layer. `PositionalEncoding` is then added to these embeddings to provide the model with information about the token order.

The core of the model is a series of `TransformerBlock` layers. Each block processes the sequence of vectors, applying `MultiHeadAttention` to identify relationships within the sequence, followed by a `FeedForward` network. `LayerNorm` is applied before and after these operations for stabilization.

Finally, the output from the last `TransformerBlock` is passed to the `Generator`. The generator, configured by a `GenerationConfig` object, uses the model's output to predict the next token in the sequence. This process is repeated to generate the desired number of new tokens, which are then decoded back into text by the `Tokenizer`.

## Python Model (`quanta_tissu/tisslm/core`)

```
+-----------------------------------------------------------------------------+
| Model (architecture/llm.py)                                                 |
|-----------------------------------------------------------------------------|
| - embeddings: Parameter                                                     |
| - pos_encoding: PositionalEncoding                                          |
| - transformer_blocks: list[TransformerBlock or ConvTransformerBlock]        |
| - output_proj: Parameter (optional, tied to embeddings)                     |
|-----------------------------------------------------------------------------|
| + forward(token_ids): logits, cache                                         |
| + backward(d_logits, cache): None                                           |
+-----------------------------------------------------------------------------+
       |
       | owns/composes
       |
       +----------------> TransformerBlock (architecture/llm.py)
       |                     |
       |                     +----> MultiHeadAttention (layers.py)
       |                     |        |
       |                     |        +-----> LoRALayer (optional)
       |                     |
       |                     +----> FeedForward or MoE (layers.py)
       |                     |        |
       |                     |        +-----> Router (for MoE)
       |                     |        +-----> Expert (for MoE)
       |                     |
       |                     +----> LayerNorm (layers.py)
       |                     |
       |                     +----> Dropout (layers.py)
       |
       +----------------> ConvTransformerBlock (architecture/llm.py) (optional)
       |                     |
       |                     +----> DepthwiseSeparableConv (convolution.py)
       |
       +----------------> PositionalEncoding (architecture/llm.py)
       |
       | uses
       v
+-----------------------------------------------------------------------------+
| Tokenizer (tokenizer.py)                                                    |
|-----------------------------------------------------------------------------|
| + encode(text): list[int]                                                   |
| + decode(ids): str                                                          |
+-----------------------------------------------------------------------------+
```

### Core Architecture Operations

The Python TISSLM model is designed for flexibility and research, centered around the `Model` class. The `forward` pass begins when a `Tokenizer` converts input text into token IDs. The `Model` takes these IDs and looks up their vector representations in the `embeddings` `Parameter`. `PositionalEncoding` is then applied to these vectors.

The resulting tensors flow through a stack of `TransformerBlock` instances. Each block performs `MultiHeadAttention` and applies a `FeedForward` network, with `LayerNorm` and `Dropout` for regularization. The architecture is highly configurable, allowing for advanced features like `LoRA` within the `MultiHeadAttention` layer for efficient fine-tuning, or a `MoE` (Mixture of Experts) layer in place of the standard `FeedForward` network. The `MoE` layer uses a `Router` to selectively engage `Expert` networks. Additionally, the standard `TransformerBlock` can be replaced with a `ConvTransformerBlock`, which uses `DepthwiseSeparableConv` instead of attention.

After the final block, the output is projected to the vocabulary size to produce logits. The `backward` pass takes the gradient of the logits and propagates it back through the model to update the `Parameter` objects.

## JavaScript Model (`quanta_tissu/tisslm/js`)

```
+-----------------------------------------------------------------------------+
| TransformerModel (transformer/TransformerModel.js)                          |
|-----------------------------------------------------------------------------|
| - embedding: Embedding                                                      |
| - posEncoding: PositionalEncoding                                           |
| - blocks: list[TransformerBlock]                                            |
| - finalNorm: LayerNorm                                                      |
| - outputLayer: Matrix                                                       |
|-----------------------------------------------------------------------------|
| + forward(tokenIds): Matrix                                                 |
+-----------------------------------------------------------------------------+
       |
       | owns/composes
       |
       +----------------> TransformerBlock (transformer/TransformerBlock.js)
       |                     |
       |                     +----> SelfAttention (transformer/SelfAttention.js)
       |                     |
       |                     +----> FeedForward (transformer/FeedForward.js)
       |                     |
       |                     +----> LayerNorm (transformer/LayerNorm.js)
       |
       +----------------> Embedding (transformer/Embeddings.js)
       |
       +----------------> PositionalEncoding (transformer/Embeddings.js)
       |
       | uses
       v
+-----------------------------------------------------------------------------+
| Tokenizer (tokenizer/Tokenizer.js)                                          |
|-----------------------------------------------------------------------------|
| + encode(text): list[int]                                                   |
| + decode(ids): str                                                          |
+-----------------------------------------------------------------------------+
```

### Core Architecture Operations

The JavaScript TISSLM model is optimized for browser and Node.js environments. The central class is `TransformerModel`. The process begins when the `Tokenizer` converts an input string into an array of token IDs.

The `forward` method of the `TransformerModel` orchestrates the main workflow. It first converts the token IDs into vectors using the `Embedding` class and then applies `PositionalEncoding`. The resulting `Matrix` object is then processed through a series of `TransformerBlock`s.

Each `TransformerBlock` contains a `SelfAttention` layer, a `FeedForward` layer, and two `LayerNorm` layers, implementing the standard transformer block architecture. After passing through all the blocks, a final `LayerNorm` is applied. The output is then multiplied by the `outputLayer` matrix to produce the final logits, which represent the model's prediction for the next token.

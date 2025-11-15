# TISSLM Model Network Diagrams

This document contains highly detailed ASCII art network state diagrams for the C++, Python, and JavaScript TISSLM models, showing class-level components and operational descriptions.

## C++ Model (`quanta_tissu/tisslm/program`)

```
+------------------------------------------+
|             1 Main (main.cpp)            |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|      2 QuantaTissu / TissuSinew          |
|    (quantatissu.cpp, tissu_sinew.cpp)    |
+--------------------+---------------------+
                     |
                     v
+------------------------------------------+
|  3 Core / Arch (core/, architecture/)    |
|  +------------------------------------+  |
|  | 3.1 Model (architecture/model.h)   |  |
|  | 3.2 TransformerModel (core/)       |  |
|  | 3.3 Matrix, 3.4 Parameter (core/)  |  |
|  | 3.5 Embedding (core/)              |  |
|  | 3.6 PositionalEncoding (core/)     |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|        4 Tokenizer (tokenizer/)          |
|  +------------------------------------+  |
|  | 4.1 Tokenizer, 4.2 PreTokenizer    |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|            5 Layers (layers/)            |
|  +------------------------------------+  |
|  | 5.1 TransformerBlock               |  |
|  | 5.2 MultiHeadAttention             |  |
|  | 5.3 FeedForward, 5.4 LayerNorm     |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+------------------------------------------+
|   6 Generation / Retrieval (generation/)   |
|  +------------------------------------+  |
|  | 6.1 Generator (generation/)        |  |
|  | 6.2 GenerationConfig (generation/) |  |
|  | 6.3 RetrievalStrategy (retrieval/) |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|       7 Pipelines / NexusFlow            |
|     (pipelines/, nexus_flow/)            |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|          8 DB / DDL Parser               |
|       (db/, ddl_parser.cpp)              |
+------------------------------------------+
```

### Core Architecture Operations

The C++ model operates as a high-performance inference engine. An incoming request is handled by `main.cpp`, which instantiates `QuantaTissu` or `TissuSinew` to manage the end-to-end process. The request payload is tokenized via the `Tokenizer` component. These tokens are converted into vector representations by the `Embedding` and `PositionalEncoding` layers within the `core` module. The `TransformerModel` then processes these embeddings through a series of `TransformerBlock` layers. Each block applies `MultiHeadAttention` and `FeedForward` networks in sequence, with `LayerNorm` applied at each step. The final output is passed to a `Generator` which, guided by a `GenerationConfig`, produces the output sequence. The process may also involve a `RetrievalStrategy` to pull in external knowledge.

## Python Model (`quanta_tissu/tisslm/core`)

```
+------------------------------------------+
|  1 Model (architecture/llm.py:Model)     |
|  +------------------------------------+  |
|  | 2 TransformerBlock                 |  |
|  |   2.1 MultiHeadAttention (layers.py)|  |
|  |   2.2 FeedForward (layers.py)      |  |
|  |   2.3 LayerNorm (layers.py)        |  |
|  |   2.4 Dropout (layers.py)          |  |
|  | 3 ConvTransformerBlock (optional)  |  |
|  |   3.1 DepthwiseSeparableConv       |  |
|  | 4 MoE (optional, layers.py)        |  |
|  |   4.1 Router, 4.2 Expert           |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
| 5 Embeddings & Positional Encoding       |
| (llm.py:PositionalEncoding)              |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|      6 Tokenizer (tokenizer.py)          |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
| 7 Core Components (parameter.py)         |
|  +------------------------------------+  |
|  | 7.1 Parameter, 7.2 LoRALayer       |  |
|  +------------------------------------+  |
+------------------------------------------+

```

### Core Architecture Operations

The Python model is designed for flexibility and research. A forward pass begins with the `Tokenizer` converting input text into tokens. The `Model` class from `llm.py` orchestrates the main workflow. It takes token IDs and passes them to an `Embedding` layer, followed by `PositionalEncoding`. The resulting tensor flows through a stack of `TransformerBlock` instances. Each block performs `MultiHeadAttention` and applies a `FeedForward` network, with `LayerNorm` and `Dropout` for regularization. The model supports advanced configurations like `MoE` (Mixture of Experts) for conditional computation and `ConvTransformerBlock` for using convolutions instead of attention. The final output is a set of logits, which can be converted into text. The `Parameter` class is a core component, tracking all learnable weights and their gradients during training.

## JavaScript Model (`quanta_tissu/tisslm/js`)

```
+------------------------------------------+
|          1 API Endpoint (api/)           |
|  +------------------------------------+  |
|  | 1.1 index.js                       |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|      2 Tokenizer (tokenizer/Tokenizer.js)|
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|   3 Model (transformer/TransformerModel.js)|
|  +------------------------------------+  |
|  | 4 TransformerBlock.js              |  |
|  |   4.1 SelfAttention.js             |  |
|  |   4.2 FeedForward.js               |  |
|  |   4.3 LayerNorm.js                 |  |
|  +------------------------------------+  |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|   5 Embeddings (transformer/Embeddings.js)|
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|      6 Core (core/Matrix.js)           |
+------------------------------------------+
```

### Core Architecture Operations

The JavaScript model is optimized for browser and server-side execution via Node.js. An incoming request, typically from a UI or an API call handled by `index.js`, is first processed by the `Tokenizer`. The resulting tokens are then fed into the `TransformerModel`. This model orchestrates the flow through its components: `Embeddings` converts tokens to vectors, which then pass through a series of `TransformerBlock`s. Each block contains `SelfAttention`, `FeedForward`, and `LayerNorm` modules. All matrix operations rely on the `Matrix.js` utility class. The final output from the last block is projected to produce the result, which is then sent back through the API. This architecture allows for in-browser inference, enabling interactive applications.

# TISSLM Model Network Diagrams

This document contains highly detailed ASCII art network state diagrams for the C++,
Python, and JavaScript TISSLM models, showing class-level components and
operational descriptions.

## C++ Model (`quanta_tissu/tisslm/program`)

```
+----------+   +--------------+   +-------------+   +-----------+
| 1 Main   | ->| 2 QuantaTissu/ | ->| 3 Core/Arch | ->| 4         |
| (main.cpp)|   | TissuSinew   |   | (core/)     |   | Tokenizer |
+----------+   +--------------+   +-------------+   +-----------+
      |
      v
+----------+   +--------------+   +-------------+   +-------------+
| 5 Layers | ->| 6 Generation/| ->| 7 Pipelines/| ->| 8 DB/DDL    |
| (layers/)  |   | Retrieval    |   | NexusFlow   |   | Parser/Mgr  |
+----------+   +--------------+   +-------------+   +-------------+
```

### Core Architecture Operations

The C++ model operates as a high-performance inference engine. An incoming
request is handled by `main.cpp`, which instantiates `QuantaTissu` or
`TissuSinew` to manage the end-to-end process. The request payload is
tokenized via the `Tokenizer` component. These tokens are converted into vector
representations by the `Embedding` and `PositionalEncoding` layers within the
`core` module. The `TransformerModel` then processes these embeddings through a
series of `TransformerBlock` layers. Each block applies `MultiHeadAttention`
and `FeedForward` networks in sequence, with `LayerNorm` applied at each step.
The final output is passed to a `Generator` which, guided by a
`GenerationConfig`, produces the output sequence. The process may also involve
a `RetrievalStrategy` to pull in external knowledge from the `retrieval`
module. The `db` module, along with the `DDLParser` and `SchemaManager`,
handles database interactions and schema management.

## Python Model (`quanta_tissu/tisslm/core`)

```
+-----------------+   +-------------------+   +-------------+
| 1 Model         | ->| 5 Embeddings &    | ->| 6 Tokenizer |
| (llm.py:Model)  |   | Positional Enc.   |   | (tokenizer.py)|
+-----------------+   +-------------------+   +-------------+
      |
      v
+-----------------+   +-------------------+
| 7 Generation/   | ->| 8 Core Components/|
| Retrieval       |   | Training          |
+-----------------+   +-------------------+
```

### Core Architecture Operations

The Python model is designed for flexibility and research. A forward pass
begins with the `Tokenizer` converting input text into tokens. The `Model`
class from `llm.py` orchestrates the main workflow. It takes token IDs and
passes them to an `Embedding` layer from the `embedding` module, followed by
`PositionalEncoding`. The resulting tensor flows through a stack of
`TransformerBlock` instances. Each block performs `MultiHeadAttention` and
applies a `FeedForward` network, with `LayerNorm` and `Dropout` for
regularization. The model supports advanced configurations like `MoE` (Mixture
of Experts) for conditional computation and `ConvTransformerBlock` for using
convolutions instead of attention. The `generation` and `retrieval` modules
handle the final output generation. The `Parameter` class is a core component,
tracking all learnable weights, and the `training` module contains logic for
optimization and scheduling.

## JavaScript Model (`quanta_tissu/tisslm/js`)

```
+----------------+   +-------------+   +-------------+
| 1 API Endpoint | ->| 2 Tokenizer | ->| 3 Model     |
| (api/)         |   | (tokenizer.js)|   | (transformer.js)|
+----------------+   +-------------+   +-------------+
      |
      v
+----------------+   +-------------+   +-------------+
| 5 Embeddings   | ->| 6 Generation| ->| 7 Core/DB   |
| (embeddings.js)|   | (generation/)|   | (matrix.js/db)|
+----------------+   +-------------+   +-------------+
```

### Core Architecture Operations

The JavaScript model is optimized for browser and server-side execution via
Node.js. An incoming request, typically from a UI or an API call handled by
`index.js`, is first processed by the `Tokenizer`. The resulting tokens are then
fed into the `TransformerModel`. This model orchestrates the flow through its
components: `Embeddings` converts tokens to vectors, which then pass through a
series of `TransformerBlock`s. Each block contains `SelfAttention`,
`FeedForward`, and `LayerNorm` modules. The `generation` module produces the
final output. All matrix operations rely on the `Matrix.js` utility class, and
the `database` module handles any data persistence. The final output from the
last block is projected to produce the result, which is then sent back through
the API. This architecture allows for in-browser inference, enabling
interactive applications.

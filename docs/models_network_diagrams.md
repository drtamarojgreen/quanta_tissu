# TISSLM Model Network Diagrams

This document contains highly detailed ASCII art network state diagrams for the C++, Python, and JavaScript TISSLM models, showing class-level components.

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
+--------------------+---------------------+
|        3 Core / Architecture             |
|          (core/, architecture/)          |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|          4 Tokenizer (tokenizer/)        |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|           5 Layers (layers/)             |
+--------------------+---------------------+
                     |
                     v
+--------------------+---------------------+
|       6 Generation / Retrieval           |
|      (generation/, retrieval/)           |
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

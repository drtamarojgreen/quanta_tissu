# TISSLM Model Network Diagrams

This document contains ASCII art network state diagrams for the C++, Python, and JavaScript TISSLM models.

## C++ Model (`tissdb`)

```
+---------------------+
|      1 API Layer    |
|      (main.cpp)     |
+----------+----------+
           |
           v
+----------+----------+
|  1.1 Query Processor|
+----------+----------+
           |
           v
+----------+----------+
|   1.1.1 Auth &      |
|     Crypto          |
+----------+----------+
           |
           v
+----------+----------+
| 1.1.1.1 Storage     |
|         Engine      |
+---------------------+
```

## Python Model (`quanta_tissu/tisslm/core`)

```
+---------------------+
|    1 Tokenizer      |
|   (tokenizer.py)    |
+----------+----------+
           |
           v
+----------+----------+
| 1.1 Embedding Layer |
+----------+----------+
           |
           v
+----------+----------+
| 1.1.1 Transformer   |
|      Layers         |
|      (layers.py)    |
+----------+----------+
           |
           v
+----------+----------+
| 1.1.1.1 Output/     |
|      Generation     |
| (generate_text.py)  |
+---------------------+
```

## JavaScript Model (`quanta_tissu/tisslm/js`)

```
+---------------------+
|      1 API/UI       |
|       (api)         |
+----------+----------+
           |
           v
+----------+----------+
|   1.1 Core Model    |
|       (core)        |
+----------+----------+
           |
           v
+----------+----------+
| 1.1.1 Tokenizer &   |
|      Transformer    |
| (tokenizer, transformer)|
+----------+----------+
           |
           v
+----------+----------+
| 1.1.1.1 Database    |
|      Interface      |
|      (database)     |
+---------------------+
```

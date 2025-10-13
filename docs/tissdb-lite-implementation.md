# TissDB-Lite Implementation Details

This document provides a detailed explanation of the internal implementation of `tissdb-lite`.

## Core Components

The `TissDBLite` class is the core of the library. It manages both the in-memory database and the query translation for Wix Data.

### In-Memory Storage

The in-memory storage is a simple object, `this.collections`, where each key is a collection name and the value is an array of documents.

```javascript
constructor() {
    this.collections = {};
}

createCollection(name) {
    this.collections[name] = [];
}
```

### Unique ID Generation

To avoid external dependencies, `tissdb-lite` uses a self-contained unique ID generator. The `_generateId` method creates a unique ID by combining a timestamp with a random string.

```javascript
_generateId() {
    const timestamp = new Date().getTime();
    const random = Math.random().toString(36).substring(2, 9);
    return `${timestamp}-${random}`;
}
```

## Query Engine

The most complex part of `tissdb-lite` is its query engine. It is designed to parse a TissLang-like query string and apply it to both the in-memory database and Wix Data collections. The engine consists of a tokenizer, a shunting-yard implementation to convert to Reverse Polish Notation (RPN), and evaluators for both in-memory and Wix Data.

### 1. Tokenizer (`_tokenize`)

The `_tokenize` method uses a regular expression to break the query string into a stream of tokens. It recognizes operators, identifiers (field names), literals (strings and numbers), and parentheses.

### 2. Shunting-Yard Algorithm (`_toRpn`)

The `_toRpn` method implements the shunting-yard algorithm to convert the infix token stream into a postfix (RPN) queue. This makes the query much easier to evaluate. It correctly handles operator precedence and associativity, as well as grouping with parentheses.

### 3. In-Memory Evaluator (`_createFilterFunction`)

The `_createFilterFunction` method takes the RPN queue and returns a filter function that can be used with `Array.prototype.filter`. It evaluates the RPN expression for each item in the in-memory collection.

### 4. Wix Data Evaluator (`_buildWixFilter`)

The `_buildWixFilter` method also consumes the RPN queue but constructs a `wix-data.WixDataFilter` object. It iterates through the RPN tokens and builds up the filter by composing Wix Data filter methods like `.eq()`, `.gt()`, `.and()`, and `.or()`.

This dual-evaluator approach allows `tissdb-lite` to use the exact same query string for both persistent Wix Data collections and ephemeral in-memory collections, providing a consistent developer experience.

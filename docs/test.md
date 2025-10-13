> [!NOTE]
> This document outlines the initial testing strategy. The implemented tests, which cover a broader range of functionalities, can be found in the `/tests` directory.

# QuantaTissu Test Plan

This document outlines the testing strategy for the QuantaTissu application, including unit tests and behavior-driven development (BDD) tests.

## 1. Unit Tests

Unit tests are written to ensure the correctness of individual components of the application. The tests can be found in `tests/unit/`.

### 1.1. Tokenizer (BPE)

-   `test_train_bpe`: Should correctly train a tokenizer from a corpus and save it.
-   `test_tokenize`: Should correctly convert a string of text into a sequence of subword token IDs.
-   `test_detokenize`: Should correctly convert a sequence of token IDs back into a string of text.
-   `test_out_of_vocabulary`: Should handle characters not seen during training.

### 1.2. Core Model Components

-   `test_softmax`, `test_layer_norm`, `test_scaled_dot_product_attention`, `test_multi_head_attention`, `test_feed_forward`, `test_positional_encoding`.
-   These tests verify the mathematical correctness and output shapes of the core transformer building blocks.

### 1.3. Training Components
-   `test_optimizer`: Verify that the Adam optimizer correctly updates model weights.
-   `test_loss`: Verify the cross-entropy loss calculation.
-   `test_kv_caching`: Verify that inference with the KV cache produces the same results as without it, but faster.

## 2. Behavior-Driven Development (BDD) Tests

BDD tests ensure the application behaves as expected from a user's perspective. The feature files and step definitions are in `tests/features/`.

### 2.1. TissLang

**Feature**: TissLang Script Execution
As a developer, I want to execute `.tiss` scripts to perform tasks.

**Scenario**: Create and test a file
**Given** a TissLang script that writes "hello" to `output.txt`
**When** I execute the script
**Then** the file `output.txt` should exist
**And** its content should be "hello"

**Scenario**: Run a command and assert its output
**Given** a TissLang script that runs `echo "test"`
**When** I execute the script
**Then** the last run result's stdout should contain "test"
**And** the last run result's exit code should be 0

### 2.2. TissDB API

**Feature**: TissDB Document Management
As a user, I want to be able to manage documents in TissDB via the REST API.

**Scenario**: Create a new document in a collection
**Given** a running TissDB instance
**When** I send a POST request to "/my_collection" with a JSON body
**Then** the response status code should be 201
**And** the response should contain the ID of the created document

**Scenario**: Retrieve an existing document
**Given** a document with ID "1234" exists in the "my_collection" collection
**When** I send a GET request to "/my_collection/1234"
**Then** the response status code should be 200
**And** the response body should contain the document's data

### 2.3. Tissu Sinew C++ Connector

**Feature**: Tissu Sinew C++ Connector
As a C++ developer, I want to reliably connect to TissDB.

**Scenario**: Successfully connect and run a PING command
**Given** a TissDB server running on "127.0.0.1" at port 9876
**When** I use the C++ connector to run the "PING" command
**Then** the connector should successfully return "PONG"

**Scenario**: Fail to connect to a non-existent server
**Given** no TissDB server is running on "127.0.0.1" at port 8081
**When** I attempt to use the C++ connector to run a "PING" command on port 8081
**Then** the connector should fail with a connection error

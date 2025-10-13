# Algorithmic RAG for TissLM using multiple_viewer

This document outlines a scalable plan for integrating the `multiple_viewer` graph visualization tool with the `quanta_tissu` text generation model, `tisslm_algorithmic_rag`. Given the large number of nodes in `multiple_viewer`, we will use a messaging system to decouple the two applications and handle the data flow efficiently.

## 1. Architecture: Custom Native Messaging System

To avoid introducing external dependencies like RabbitMQ or ZeroMQ, we will implement a custom, high-performance native messaging system for Inter-Process Communication (IPC) between `multiple_viewer` and `quanta_tissu`. This system will be based on shared memory.

### Components

1.  **Shared Memory Buffer:** A block of shared memory will be allocated to serve as a circular buffer. This buffer will hold the messages (node data) being passed from the producer to the consumer.
2.  **Synchronization Primitives:** To manage access to the shared buffer and prevent race conditions, we will use native synchronization objects:
    *   **Mutex:** A mutex will provide exclusive access to the shared buffer, ensuring that only one process can write to or read from it at any given time.
    *   **Semaphores (or Condition Variables):** Two semaphores will be used to signal the state of the buffer:
        *   `empty_count`: Tracks the number of empty slots in the buffer. The producer will wait on this semaphore if the buffer is full.
        *   `filled_count`: Tracks the number of filled slots. The consumer will wait on this semaphore if the buffer is empty.

This design allows for efficient, lock-free communication when the buffer is not full or empty, as the producer and consumer will only block when necessary.

## 2. Message Format

The data for each node will be serialized into a JSON string before being placed into the shared memory buffer.

```json
{
  "id": "node-1",
  "label": "Node 1 Label",
  "properties": {
    "property1": "value1",
    "property2": "value2"
  }
}
```

## 3. `multiple_viewer` (Producer) Logic

1.  Wait for an empty slot in the buffer by waiting on the `empty_count` semaphore.
2.  Acquire the mutex to gain exclusive access to the buffer.
3.  Write the serialized node data (JSON string) to the next available slot in the circular buffer.
4.  Release the mutex.
5.  Signal the `filled_count` semaphore to indicate that a new message is available.

## 4. `quanta_tissu` (Consumer) Logic

1.  Run a background thread that continuously listens for new messages.
2.  Wait for a message to be available by waiting on the `filled_count` semaphore.
3.  Acquire the mutex to gain exclusive access to the buffer.
4.  Read the serialized node data from the next slot in the circular buffer.
5.  Release the mutex.
6.  Signal the `empty_count` semaphore to indicate that a slot has been freed.
7.  Parse the JSON data and update the local, in-memory data store.

## 5. RAG Integration

The RAG integration remains the same as in the previous plan. The model will:

1.  Query the local data store (which is populated by the consumer thread) for relevant nodes.
2.  Format the retrieved node data into a textual representation.
3.  Prepend this text to the user's query as context for the LLM.

## 6. Data Flow

1.  `multiple_viewer` generates a graph, serializes each node to JSON, and writes it to the shared memory buffer.
2.  `quanta_tissu`'s background consumer thread reads the data from the shared memory buffer.
3.  The consumer thread parses the JSON and populates a local, in-memory data store.
4.  A user sends a query to `quanta_tissu`.
5.  The RAG model queries the local data store.
6.  The retrieved data is formatted and passed to the LLM along with the query.

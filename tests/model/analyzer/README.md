# Runtime Model Analyzer (RMA) Framework

A dual-process error monitoring framework for C++ applications, designed to detect and report runtime arithmetic anomalies (like division by zero) without interrupting the primary application flow.

## Architecture

The RMA framework consists of two main processes:
1.  **Host Application**: The model or application being monitored. It uses the `ErrorHandler` singleton to report errors.
2.  **Analyzer Process**: A standalone terminal application that receives, filters, and logs error messages from the Host.

Communication is handled via **POSIX Shared Memory IPC**, using a ring-buffer protocol with data checksums for integrity.

## Components

-   `error_handler.hpp`: Singleton interface for the host application to report errors.
-   `rpc_protocol.hpp`: Low-level IPC implementation using shared memory.
-   `analyzer_queue.hpp`: Thread-safe scheduling queue for processing received errors.
-   `filter_manager.hpp`: Keyboard-driven filtering logic for the analyzer UI.
-   `file_buffer.hpp`: Persistent logging of errors to a file.
-   `benchmark.hpp`: Performance metrics for monitoring IPC and processing overhead.

## Build Instructions

To build the analyzer and the example host application:

```bash
cd tests/model/analyzer
make all
```

To run the unit tests:

```bash
make unit_tests
./unit_tests
```

## Usage

### 1. Start the Analyzer
In one terminal, start the analyzer process. You can specify a session ID and an output log file.

```bash
./analyzer -s 42 -o analyzer_log.txt
```

### 2. Start the Host Application
In another terminal, start your application (or the provided example).

```bash
./host_example 42
```

### 3. Interactive Controls
The Analyzer UI supports real-time filtering and status reports via keyboard input:
-   `0-9, a-e`: Toggle specific error type filters.
-   `A / Z`: Enable / Disable all filters.
-   `H`: Show help and filter status.
-   `B`: Print performance benchmark report.
-   `F`: Flush file buffer.
-   `Q`: Graceful shutdown.

## Integration Guide

To instrument a new application:
1.  Include `tests/model/analyzer/error_handler.hpp`.
2.  Initialize the handler at startup: `RMA_INIT(session_id);`.
3.  Report errors using macros:
    -   `RMA_ERROR_VAL(type, value, message)`: Report an error with a typed value.
    -   `RMA_ERROR_FMT(type, format, ...)`: Report an error with a formatted string.
    -   `RMA_DEBUG(message)`: Log a debug message.
    -   `RMA_NULL_CHECK(ptr)`: Automatically report if a pointer is null.

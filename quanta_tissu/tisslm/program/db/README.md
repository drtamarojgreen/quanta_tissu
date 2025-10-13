# TissDB C++ Client Implementation

This directory contains the C++ implementation of the TissDB client for interacting with the TissDB HTTP API.

## Components

### TissDBClient (`tissdb_client.h`, `tissdb_client.cpp`)
A C++ client for interacting with TissDB running on localhost:9876. Features include:
- Database and collection creation
- Document storage and retrieval
- Feedback collection
- Database statistics

### Document Structure
The `Document` class provides a simple key-value structure for storing data:
- Fields stored as string key-value pairs
- JSON serialization/deserialization
- Unique document IDs

## Dependencies

### Required
- **libcurl**: HTTP client library for making REST API calls
  - Windows: Download from https://curl.se/windows/
  - Linux: `sudo apt-get install libcurl4-openssl-dev`
  - macOS: `brew install curl`

### Alternative Implementation
If libcurl is not available, you can use:
- Windows: WinHTTP API (native)
- Cross-platform: cpp-httplib (header-only library)
- Python binding: Use pybind11 to wrap the Python client

## Building

### With CMake (when libcurl is available)

```bash
cd quanta_tissu/tisslm/program/db
mkdir build && cd build
cmake ..
cmake --build .
```

### CMakeLists.txt Example

```cmake
cmake_minimum_required(VERSION 3.10)
project(TissDBClient)

set(CMAKE_CXX_STANDARD 17)

# Find libcurl
find_package(CURL REQUIRED)

# Add source files
add_library(tissdb_client tissdb_client.cpp)
target_include_directories(tissdb_client PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(tissdb_client ${CURL_LIBRARIES})

# Test executable
add_executable(test_tissdb_client test_tissdb_client.cpp)
target_link_libraries(test_tissdb_client tissdb_client)
```

## Usage Example

```cpp
#include "tissdb_client.h"
#include <iostream>

using namespace TissDB;

int main() {
    try {
        // Connect to TissDB
        TissDBClient client("127.0.0.1", 9876, "mydb");
        
        // Setup database
        std::vector<std::string> collections = {"documents", "embeddings"};
        client.ensure_db_setup(collections);
        
        // Add a document
        Document doc;
        doc.set_field("title", "My Document");
        doc.set_field("content", "Document content here");
        
        std::string doc_id = client.add_document("documents", doc);
        std::cout << "Document added: " << doc_id << std::endl;
        
        // Retrieve document
        Document retrieved = client.get_document("documents", doc_id);
        std::cout << "Title: " << retrieved.get_field("title") << std::endl;
        
    } catch (const DatabaseException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
```

## Testing with Running TissDB Instance

Ensure TissDB is running on localhost:9876 before running tests:

```bash
# Run the test program
./test_tissdb_client
```

Expected output:
```
TissDB C++ Client Test
======================

=== Testing TissDB Connection ===
Testing connection...
✓ Connection successful

Setting up database and collections...
✓ Database setup successful

Adding test document...
✓ Document added with ID: <uuid>

Retrieving document...
✓ Document retrieved:
  Title: Test Document
  Content: This is a test document from C++
  Type: test

=== All tests passed! ===
```

## Integration with Retrieval System

The TissDB client is designed to work with the retrieval strategies in `quanta_tissu/tisslm/program/retrieval/`:
- Store document embeddings in the database
- Retrieve documents for similarity search
- Track feedback for improving retrieval quality

## Future Enhancements

1. **Batch Operations**: Add support for bulk document insertion
2. **Query Language**: Implement TissDB query language support
3. **Async Operations**: Add asynchronous API calls
4. **Connection Pooling**: Implement connection pooling for better performance
5. **Caching**: Add local caching layer for frequently accessed documents

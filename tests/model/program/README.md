# TissLM C++ Test Suite

Comprehensive unit and integration tests for the TissLM C++ implementation, including database operations and retrieval strategies.

## Test Coverage

### Database Tests (5 tests)
1. **Database Connection** - Tests connection to TissDB on 127.0.0.1:9876
2. **Database Setup** - Tests database and collection creation
3. **Document CRUD** - Tests create, read operations for documents
4. **Feedback Collection** - Tests feedback document storage
5. **Multiple Documents** - Tests batch document operations

### Retrieval Strategy Tests (5 tests)
1. **Cosine Similarity** - Tests cosine similarity calculations
2. **Euclidean Distance** - Tests distance-based similarity
3. **Dot Product** - Tests dot product similarity
4. **BM25** - Tests keyword-based retrieval
5. **Hybrid Strategy** - Tests weighted combination of strategies

### Integration Tests (2 tests)
1. **Database with Embeddings** - Tests storing embedding metadata
2. **Complete Retrieval Pipeline** - Tests end-to-end retrieval workflow

## Prerequisites

### Required
- **TissDB** running on 127.0.0.1:9876
- **libcurl** for HTTP requests
- **CMake** 3.10 or higher
- **C++17** compatible compiler

### Starting TissDB
Ensure TissDB is running before executing tests:
```bash
# Check if TissDB is running
curl http://127.0.0.1:9876

# If not running, start TissDB (adjust path as needed)
./tissdb --port 9876
```

## Building Tests

### Using CMake

```bash
cd tests/model/program
mkdir build && cd build
cmake ..
cmake --build .
```

### Build Output
- `test_db_and_retrieval` - Main test executable
- `libtissdb_client.a` - Database client library
- `libretrieval_strategies.a` - Retrieval strategies library

## Running Tests

### Run All Tests
```bash
cd build
./test_db_and_retrieval
```

### Using CTest
```bash
cd build
ctest --verbose
```

### Expected Output
```
============================================================
TissLM C++ Database and Retrieval Test Suite
Testing against TissDB on 127.0.0.1:9876
============================================================

=== Testing Database Connection ===
  ✓ Database connection

=== Testing Database Setup ===
  ✓ Database and collection creation

=== Testing Document CRUD Operations ===
  ✓ Document creation
  ✓ Document retrieval

... (more tests)

============================================================
Test Summary
============================================================
Passed: 12
Failed: 0
Total:  12
============================================================
```

## Test Details

### Database Tests

#### Connection Test
- Verifies HTTP connection to TissDB
- Tests basic connectivity
- Validates server response

#### Setup Test
- Creates test database
- Creates multiple collections
- Handles existing database/collections

#### CRUD Test
- Creates document with multiple fields
- Retrieves document by ID
- Validates field values match

#### Feedback Test
- Stores feedback with timestamp
- Validates feedback ID format
- Tests feedback collection

#### Multiple Documents Test
- Creates 5 documents in batch
- Retrieves all documents
- Validates data integrity

### Retrieval Strategy Tests

#### Cosine Similarity Test
- Tests with orthogonal vectors (similarity = 0)
- Tests with identical vectors (similarity = 1)
- Tests with angled vectors (similarity ≈ 0.707)

#### Euclidean Distance Test
- Tests with identical points (distance = 0)
- Tests with unit distance
- Validates inverse relationship to similarity

#### Dot Product Test
- Tests with various vector combinations
- Validates mathematical correctness
- Tests with zero vectors

#### BM25 Test
- Tests keyword-based search
- Validates TF-IDF calculations
- Tests with multi-word queries

#### Hybrid Strategy Test
- Combines cosine and Euclidean strategies
- Tests weighted combination (0.7 + 0.3)
- Validates score normalization

### Integration Tests

#### Database with Embeddings
- Stores documents with embedding metadata
- Tests embedding dimension tracking
- Validates metadata retrieval

#### Complete Retrieval Pipeline
- Simulates full retrieval workflow
- Tests query embedding against document embeddings
- Validates best match selection

## Troubleshooting

### Connection Failures
```
✗ Database connection - Failed to connect to TissDB
```
**Solution:** Ensure TissDB is running on 127.0.0.1:9876

### Build Errors
```
Could not find CURL
```
**Solution:** Install libcurl development package
- Windows: Download from https://curl.se/windows/
- Linux: `sudo apt-get install libcurl4-openssl-dev`
- macOS: `brew install curl`

### Test Failures
If tests fail, check:
1. TissDB is running and accessible
2. No firewall blocking port 9876
3. Sufficient permissions for database operations
4. Previous test data doesn't conflict

## Continuous Integration

### Running in CI/CD
```bash
# Start TissDB in background
./tissdb --port 9876 &
TISSDB_PID=$!

# Wait for TissDB to be ready
sleep 2

# Run tests
cd quanta_tissu/tisslm/program/tests/build
./test_db_and_retrieval

# Cleanup
kill $TISSDB_PID
```

### Test Exit Codes
- `0` - All tests passed
- `1` - One or more tests failed

## Adding New Tests

### Test Function Template
```cpp
void test_new_feature() {
    std::cout << "\n=== Testing New Feature ===" << std::endl;
    
    try {
        // Test implementation
        
        if (/* success condition */) {
            results.record_pass("New feature test");
        } else {
            results.record_fail("New feature test", "Reason");
        }
    } catch (const std::exception& e) {
        results.record_fail("New feature test", e.what());
    }
}
```

### Register Test in main()
```cpp
int main() {
    // ... existing tests ...
    test_new_feature();
    // ...
}
```

## Performance Benchmarks

Typical test execution times (with TissDB running):
- Database tests: ~500ms
- Retrieval tests: ~50ms
- Integration tests: ~200ms
- **Total: ~750ms**

## Test Data Cleanup

Tests create data in the `test_cpp_db` database. To clean up:

```bash
# Using TissDB CLI or API
curl -X DELETE http://127.0.0.1:9876/test_cpp_db
```

## Related Documentation

- [TissDB Client README](../../../quanta_tissu/tisslm/program/db/README.md)
- [C++ Implementation Progress](../../../docs/tisslm_cpp_progress.md)
- [TissDB API Documentation](https://github.com/tissdb/tissdb)

# TissDB Makefile
# Uses user-provided g++ command and builds in the build/ directory

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -march=native -I. -I../quanta_tissu/tisslm/program -I..
LDFLAGS = -latomic -lpthread

BUILD_DIR = build
TARGET = $(BUILD_DIR)/tissdb
TEST_TARGET = $(BUILD_DIR)/test_tissdb

# Source files for the main binary
SRCS = main.cpp \
       api/http_server.cpp \
       audit/audit_logger.cpp \
       auth/rbac.cpp \
       auth/token_manager.cpp \
       common/binary_stream_buffer.cpp \
       common/checksum.cpp \
       common/document.cpp \
       common/schema_validator.cpp \
       common/serialization.cpp \
       crypto/kms.cpp \
       json/json.cpp \
       query/executor.cpp \
       query/executor_common.cpp \
       query/executor_delete.cpp \
       query/executor_insert.cpp \
       query/executor_select.cpp \
       query/executor_update.cpp \
       query/join_algorithms.cpp \
       query/parser.cpp \
       storage/collection.cpp \
       storage/database_manager.cpp \
       storage/indexer.cpp \
       storage/lsm_tree.cpp \
       storage/memtable.cpp \
       storage/native_b_tree.cpp \
       storage/sstable.cpp \
       storage/transaction_manager.cpp \
       storage/wal.cpp \
       ../quanta_tissu/tisslm/program/ddl_parser.cpp \
       ../quanta_tissu/tisslm/program/schema_manager.cpp \
       ../quanta_tissu/tisslm/program/tissu_sinew.cpp \
       ../tests/db/http_client.cpp

# Filter out main components to build the test suite
TEST_SRCS = $(filter-out main.cpp api/http_server.cpp, $(SRCS)) ../tests/db/test_main.cpp

# Default target
all: $(TARGET)

# Build the main database binary
$(TARGET): $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

# Run the database
run: all
	./$(TARGET)

# Build and run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(LDFLAGS) -o $(TEST_TARGET)

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run test analysis

# Analysis rule
ANALYSIS_SRCS = analysis/ACID_analysis.cpp \
                analysis/relational_analysis.cpp \
                analysis/schema_analysis.cpp \
                analysis/transaction_analysis.cpp
ANALYSIS_TARGETS = $(patsubst analysis/%.cpp, $(BUILD_DIR)/%, $(ANALYSIS_SRCS))

analysis: $(ANALYSIS_TARGETS)
	@echo "\n\n--- Running TissDB Analysis ---"
	@for target in $(ANALYSIS_TARGETS); do ./$$target; done
	@echo "\n--- Analysis Complete ---\n"

$(BUILD_DIR)/%: analysis/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

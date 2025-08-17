# TissDB Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -I.. -I. -Wall -Wextra -g

# Source files
SRCS = main.cpp \
       api/http_server.cpp \
       common/binary_stream_buffer.cpp \
       common/schema_validator.cpp \
       common/serialization.cpp \
       json/json.cpp \
       query/executor.cpp \
       query/join_algorithms.cpp \
       query/parser.cpp \
       storage/collection.cpp \
       storage/indexer.cpp \
       storage/lsm_tree.cpp \
       storage/memtable.cpp \
       storage/native_b_tree.cpp \
       storage/sstable.cpp \
       storage/transaction_manager.cpp \
       storage/wal.cpp \
       storage/collection.cpp \
       ../tests/db/http_client.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Filter out objects that should not be in the test build
TEST_OBJS = $(filter-out main.o api/http_server.o, $(OBJS))

# Executable name
TARGET = tissdb
TEST_TARGET = test_tissdb

# Default rule
all: $(TARGET)

# Rule to link the object files into the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile the source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test rule
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# All test objects will be placed in the current directory
TEST_MAIN_SRC = ../tests/db/test_main.cpp
TEST_MAIN_OBJ = test_main.o

$(TEST_TARGET): $(TEST_OBJS) $(TEST_MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS) $(TEST_MAIN_OBJ)

$(TEST_MAIN_OBJ): $(TEST_MAIN_SRC)
	$(CXX) $(CXXFLAGS) -I.. -c $(TEST_MAIN_SRC) -o $(TEST_MAIN_OBJ)

# Rule to clean the compiled files
clean:
	-rm -f $(OBJS) $(TARGET) $(TEST_TARGET) *.o

# Rule to run the executable
run: all
	./$(TARGET)

.PHONY: all clean run test analysis

# Analysis rule
ANALYSIS_SRCS = analysis/ACID_analysis.cpp \
                analysis/relational_analysis.cpp \
                analysis/schema_analysis.cpp \
                analysis/transaction_analysis.cpp
ANALYSIS_TARGETS = $(ANALYSIS_SRCS:.cpp=)

analysis: $(ANALYSIS_TARGETS)
	@echo "\n\n--- Running TissDB Analysis ---"
	@./analysis/ACID_analysis
	@./analysis/relational_analysis
	@./analysis/schema_analysis
	@./analysis/transaction_analysis
	@echo "\n--- Analysis Complete ---\n"
	@rm -f $(ANALYSIS_TARGETS)

analysis/ACID_analysis: analysis/ACID_analysis.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

analysis/relational_analysis: analysis/relational_analysis.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

analysis/schema_analysis: analysis/schema_analysis.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

analysis/transaction_analysis: analysis/transaction_analysis.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

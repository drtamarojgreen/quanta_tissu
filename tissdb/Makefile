
# TissDB Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -I. -Wall -Wextra -g

# Source files
SRCS = main.cpp \
       api/http_server.cpp \
       common/serialization.cpp \
       json/json.cpp \
       query/executor.cpp \
       query/parser.cpp \
       storage/indexer.cpp \
       storage/lsm_tree.cpp \
       storage/memtable.cpp \
       storage/sstable.cpp \
       storage/wal.cpp \
       storage/btree.cpp \
       storage/collection.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

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

$(TEST_TARGET): $(OBJS) tests/db/test_main.o \
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(OBJS) tests/db/test_main.o

tests/db/test_main.o: tests/db/test_main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean the compiled files
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET) tests/db/*.o

# Rule to run the executable
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run test

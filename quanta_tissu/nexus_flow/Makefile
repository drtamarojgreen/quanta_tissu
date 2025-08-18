# Makefile for the Nexus Flow application

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Executable name
TARGET = nexus_flow

# Source files
SRCS = main.cpp \
       graph_logic.cpp \
       ../../../tissdb/json/json.cpp \
       ../../../tests/db/http_client.cpp

# Include paths
INCLUDES = -I. \
           -I../../../tissdb/json \
           -I../../../tests/db

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

# Clean rule
clean:
	rm -f $(TARGET)

.PHONY: all clean

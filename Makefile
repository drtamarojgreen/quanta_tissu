# Makefile for the simple App example

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# The final executable
TARGET = app_runner

# Source file
SRCS = App.cpp

# Default target
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up generated files
clean:
	rm -f $(TARGET)

.PHONY: all clean

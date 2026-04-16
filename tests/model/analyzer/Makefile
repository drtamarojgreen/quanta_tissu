CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
LDFLAGS := -pthread

# Platform-specific
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    LDFLAGS += -lrt
endif

# Targets
ANALYZER := analyzer
HOST_EXAMPLE := host_example
UNIT_TESTS := unit_tests

# Headers
HEADERS := error_types.hpp error_wrapper.hpp rpc_protocol.hpp \
           analyzer_queue.hpp filter_manager.hpp file_buffer.hpp \
           benchmark.hpp keyboard_input.hpp error_handler.hpp

.PHONY: all clean

all: $(ANALYZER) $(HOST_EXAMPLE) $(UNIT_TESTS)

$(ANALYZER): analyzer_main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(HOST_EXAMPLE): host_example.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(UNIT_TESTS): unit_tests.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(ANALYZER) $(HOST_EXAMPLE) $(UNIT_TESTS)

# Debug build
debug: CXXFLAGS += -g -DDEBUG -O0
debug: all

# Run example (start host first, then analyzer in another terminal)
run-host: $(HOST_EXAMPLE)
	./$(HOST_EXAMPLE) 0

run-analyzer: $(ANALYZER)
	./$(ANALYZER) -s 0

help:
	@echo "Targets:"
	@echo "  all          - Build analyzer and host example"
	@echo "  debug        - Build with debug symbols"
	@echo "  clean        - Remove built files"
	@echo "  run-host     - Run example host application"
	@echo "  run-analyzer - Run analyzer (connect to host)"
	@echo ""
	@echo "Usage:"
	@echo "  1. Terminal 1: make run-host"
	@echo "  2. Terminal 2: make run-analyzer"

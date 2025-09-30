cmake_minimum_required(VERSION 3.10)
project(TissLMCore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add all source files
set(CORE_SOURCES
    matrix.cpp
    parameter.cpp
    layernorm.cpp
    dropout.cpp
    feedforward.cpp
    multiheadattention.cpp
    transformerblock.cpp
    positionalencoding.cpp
    embedding.cpp
)

# Create a library from the core components
add_library(tisslm_core ${CORE_SOURCES})

# Add the test executable
add_executable(test_new_components test_new_components.cpp)

# Link the test executable with the core library
target_link_libraries(test_new_components tisslm_core)

# Enable testing
enable_testing()
add_test(NAME NewComponentsTest COMMAND test_new_components)

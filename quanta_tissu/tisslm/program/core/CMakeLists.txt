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
    configurableattention.cpp
    transformerblock.cpp
    positionalencoding.cpp
    embedding.cpp
)

# Create a library from the core components
add_library(tisslm_core ${CORE_SOURCES})

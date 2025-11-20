cmake_minimum_required(VERSION 3.10)
project(TissLM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define include directories
include_directories(quanta_tissu/tisslm/program/core)
include_directories(quanta_tissu/tisslm/program/training)
include_directories(quanta_tissu/tisslm/program/generation)
include_directories(quanta_tissu/tisslm/program/tokenizer)

# Define the library source files
set(CORE_SOURCES
    quanta_tissu/tisslm/program/core/matrix.cpp
    quanta_tissu/tisslm/program/core/parameter.cpp
    quanta_tissu/tisslm/program/core/layernorm.cpp
    quanta_tissu/tisslm/program/core/dropout.cpp
    quanta_tissu/tisslm/program/core/feedforward.cpp
    quanta_tissu/tisslm/program/core/configurableattention.cpp
    quanta_tissu/tisslm/program/core/transformerblock.cpp
    quanta_tissu/tisslm/program/core/positionalencoding.cpp
    quanta_tissu/tisslm/program/core/embedding.cpp
)

# Create the library
add_library(tisslm_core ${CORE_SOURCES})

# Define the test executables
# ... (rest of the file is the same)

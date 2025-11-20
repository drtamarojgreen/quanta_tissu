cmake_minimum_required(VERSION 3.10)
project(TissLMTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TOKENIZER_PATH_FROM_CMAKE "Path to the tokenizer files" "")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/core)
# ... (rest of the file is the same)

add_library(tisslm_core
    ../../../quanta_tissu/tisslm/program/core/matrix.cpp
    ../../../quanta_tissu/tisslm/program/core/parameter.cpp
    ../../../quanta_tissu/tisslm/program/core/layernorm.cpp
    ../../../quanta_tissu/tisslm/program/core/dropout.cpp
    ../../../quanta_tissu/tisslm/program/core/feedforward.cpp
    ../../../quanta_tissu/tisslm/program/core/configurableattention.cpp
    ../../../quanta_tissu/tisslm/program/core/transformerblock.cpp
    # ... (rest of the sources are the same)
)

# ... (rest of the CMakeLists.txt is the same)

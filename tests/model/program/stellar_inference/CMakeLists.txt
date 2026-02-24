cmake_minimum_required(VERSION 3.10)
project(StellarInference)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    .
    ../../../../quanta_tissu/tisslm/program
    ../../../../quanta_tissu/tisslm/program/layers
    ../../../../quanta_tissu/tisslm/program/architecture
    ../../../../quanta_tissu/tisslm/program/tokenizer
)

# TissLM Program Sources (Set B Indigenous Stack)
set(PROGRAM_SOURCES
    ../../../../quanta_tissu/tisslm/program/layers/matrix.cpp
    ../../../../quanta_tissu/tisslm/program/layers/feedforward.cpp
    ../../../../quanta_tissu/tisslm/program/layers/layernorm.cpp
    ../../../../quanta_tissu/tisslm/program/layers/multiheadattention.cpp
    ../../../../quanta_tissu/tisslm/program/layers/transformerblock.cpp
    ../../../../quanta_tissu/tisslm/program/architecture/model.cpp
    ../../../../quanta_tissu/tisslm/program/quantatissu.cpp
    ../../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp
    ../../../../quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp
    ../../../../quanta_tissu/tisslm/program/ddl_parser.cpp
    ../../../../quanta_tissu/tisslm/program/schema_manager.cpp
)

# Stellar Sources
set(STELLAR_SOURCES
    stellar_visualizer.cpp
    stellar_meta_analyst.cpp
    stellar_reporter.cpp
)

add_executable(stellar_inference_test
    stellar_inference_test.cpp
    ${STELLAR_SOURCES}
    ${PROGRAM_SOURCES}
)

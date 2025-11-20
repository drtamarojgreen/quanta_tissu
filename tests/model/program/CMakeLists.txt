cmake_minimum_required(VERSION 3.10)
project(TissLMTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the library source files
set(CORE_SOURCES
    ../../../quanta_tissu/tisslm/program/core/matrix.cpp
    ../../../quanta_tissu/tisslm/program/core/parameter.cpp
    ../../../quanta_tissu/tisslm/program/core/layernorm.cpp
    ../../../quanta_tissu/tisslm/program/core/dropout.cpp
    ../../../quanta_tissu/tisslm/program/core/feedforward.cpp
    ../../../quanta_tissu/tisslm/program/core/configurableattention.cpp
    ../../../quanta_tissu/tisslm/program/core/transformerblock.cpp
    ../../../quanta_tissu/tisslm/program/core/positionalencoding.cpp
    ../../../quanta_tissu/tisslm/program/core/embedding.cpp
)

# Create the library
add_library(tisslm_core ${CORE_SOURCES})

# Define the test executables
add_executable(matrix_test matrix_test.cpp)
add_executable(test_new_components test_new_components.cpp)

# Link the test executables against the library
target_link_libraries(matrix_test tisslm_core)
target_link_libraries(test_new_components tisslm_core)

# Define the tests
enable_testing()
add_test(NAME MatrixTests COMMAND matrix_test)
add_test(NAME NewComponentsTests COMMAND test_new_components)

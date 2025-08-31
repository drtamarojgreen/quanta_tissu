cmake_minimum_required(VERSION 3.10)

project(ctisslm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ctisslm_lib ctisslm.cpp ctisslm.h)

# If you want to create an executable for testing, uncomment the following lines:
# add_executable(ctisslm_test main.cpp)
# target_link_libraries(ctisslm_test ctisslm_lib)
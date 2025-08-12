cmake_minimum_required(VERSION 3.11) # FetchContent works best with 3.11+
project(TissDB CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Add msgpack-c dependency ---
include(FetchContent)
FetchContent_Declare(
  msgpack
  GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
  GIT_TAG        cpp-4.1.0 # A recent, stable version
)
FetchContent_MakeAvailable(msgpack)
# --------------------------------

# In the future, we will add subdirectories for libraries and executables
# add_subdirectory(api)
# add_subdirectory(common)
# add_subdirectory(query)
# add_subdirectory(storage)

# add_executable(tissdb_server main.cpp)

# target_link_libraries(tissdb_server
#   PRIVATE
#     api
#     common
#     query
#     storage
# )

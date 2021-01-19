project("b-link")
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRERD)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads)

include_directories($(CMAKE_CURRENT_SOURCE_DIR)/source)

add_executable(b-link source/main.cpp)
target_link_libraries(b-link Threads::Threads)

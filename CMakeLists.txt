cmake_minimum_required(VERSION 3.11)
project(Parallel_Game_of_Life_)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(lib/sdsl-lite)

add_executable(Parallel_Game_of_Life_ main.cpp)
target_link_libraries(Parallel_Game_of_Life_ sdsl divsufsort divsufsort64 pthread)
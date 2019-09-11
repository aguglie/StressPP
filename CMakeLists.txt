cmake_minimum_required(VERSION 3.6)
project(stresspp)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Add -O0 to remove optimizations when using gcc
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

add_executable(stresspp main.cpp StressPP.cpp StressPP.h StressCpuThread.cpp StressCpuThread.h)
target_link_libraries(stresspp PUBLIC Threads::Threads)
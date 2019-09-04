cmake_minimum_required(VERSION 3.6)
project(untitled)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

add_executable(untitled main.cpp StressPP.cpp StressPP.h StressCpuThread.cpp StressCpuThread.h)
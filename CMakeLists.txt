cmake_minimum_required(VERSION 3.12)
project(hermes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(shared)
add_subdirectory(synth)
add_subdirectory(gui)
cmake_minimum_required(VERSION 3.12)
project(synth)

# set variables
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # create compile commands json for vsCode

# debug
# set(CMAKE_BUILD_TYPE Debug)

# Compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -march=native -Wall -Wextra -Wpedantic -ffast-math")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG -Wall -Wextra -Wpedantic -ffast-math") # for debugging
# Find and include libraries
find_package(PkgConfig REQUIRED)

pkg_check_modules(RTAUDIO REQUIRED rtaudio)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(KISSFFT REQUIRED kissfft-float)

file(GLOB SRC "${SRC_DIR}/*.cpp" "${SRC_DIR}/*/*.cpp")

add_executable(synth ${SRC})

target_include_directories(synth PRIVATE
    ${SRC_DIR}
    ${RTAUDIO_INCLUDE_DIRS}
    ${RTMIDI_INCLUDE_DIRS}
)

target_link_libraries(synth PRIVATE 
    shared
    ${RTAUDIO_LIBRARIES}
    ${RTMIDI_LIBRARIES}
    ${KISSFFT_LIBRARIES}
    m
)

# Linker Flags
target_link_options(synth PRIVATE
    -Wl,-Bstatic
    -Wl,-Bdynamic
    -Wl,--as-needed
    -pthread
)

message(STATUS "SRC_DIR: ${SRC_DIR}")
message(STATUS "SHARED_DIR: ${SHARED_DIR}")
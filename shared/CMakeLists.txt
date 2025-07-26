cmake_minimum_required(VERSION 3.12)
project(shared)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O2 -ffast-math")

file(GLOB SHARED_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp")

add_library(shared STATIC ${SHARED_SRC})

target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET shared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.json
    $<TARGET_FILE_DIR:shared>/config.json
)

cmake_minimum_required(VERSION 3.17)
project(Chip8 C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_subdirectory(interpreter)
add_executable(Chip8 main.c)

target_link_libraries(Chip8 interpreter)
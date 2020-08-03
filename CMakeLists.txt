cmake_minimum_required(VERSION 3.17)
project(Chip8 C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_subdirectory(interpreter)
add_executable(Chip8 main.c)

target_link_libraries(Chip8 interpreter)

# add to turn on aggressive optimizations you can also add -O1 -O2 or -O3 to ADD_COMPILE_OPTIONS
# or -Ofast (super fast) or -Oz (small code and optimized)
# -O3 being the most aggressive
ADD_COMPILE_OPTIONS(-Wall -Werror -fsanitize=address)
ADD_LINK_OPTIONS(-fsanitize=address)
add_library(interpreter interpreter.c interpreter.h)

add_subdirectory(gfx)

target_include_directories(interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(interpreter gfx)
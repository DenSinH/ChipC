add_library(gfx window.c window.h)

target_link_libraries(gfx PRIVATE ${SDL2_LIBRARY} -lmingw32 -lSDL2main -lSDL2)

target_include_directories(gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <SDL2/SDL_events.h>

#include "interpreter.h"
#include "gfx/window.h"

#define ROM_NAME "./roms/AstroDodge.ch8"

s_interpreter* init_interpreter() {
    s_interpreter* _interpreter = malloc(sizeof(s_interpreter));
    memset(_interpreter, 0x00, sizeof(s_interpreter));

    return _interpreter;
}

void open_rom(s_interpreter* interpreter, char file_name[]) {
    FILE* file = fopen(file_name, "rb");
    if (!file) perror("failed read"), exit(1);

    fseek(file, 0, SEEK_END);
    int fsize = ftell(file);
    rewind(file);

    fread(interpreter->mem + 0x200, 1, fsize, file);
    fclose(file);

    printf("Read %i bytes\n", fsize);
}

int run(s_interpreter* interpreter) {
    init_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    open_rom(interpreter, ROM_NAME);

    bool quit = false;
    while (!quit) {
        SDL_Event e;

        while (SDL_PollEvent( &e ) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    printf("%c", SDL_GetKeyFromScancode(e.key.keysym.scancode));
                default:
                    break;
            }
        }
    }

    close_display();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <SDL2/SDL_events.h>
#include <time.h>

#include "interpreter.h"
#include "gfx/window.h"

#define LIMIT_FRAMERATE

#define ROM_NAME "./roms/SpaceInvaders.ch8"
#define ON_COLOR 0xffffffffu
#define OFF_COLOR 0x00000000u
#define INSTRS_PER_FRAME 10

char keys[] = "x123qweasdzc4rfv";
const struct timespec frame_delay = {.tv_sec = 0, .tv_nsec = 16000000 };  // 16ms
const SDL_Rect display_rect = {.w = WIDTH, .h = HEIGHT, .x = 0, .y = 0};

unsigned char digits[5 * 16] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xf0, 0x10, 0xf0, 0x80, 0xf0,
    0xf0, 0x10, 0xf0, 0x10, 0xf0,
    0x90, 0x90, 0xf0, 0x10, 0x10,
    0xf0, 0x80, 0xf0, 0x10, 0xf0,
    0xf0, 0x80, 0xf0, 0x90, 0xf0,
    0xf0, 0x10, 0x20, 0x40, 0x40,
    0xf0, 0x90, 0xf0, 0x90, 0xf0,
    0xf0, 0x90, 0xf0, 0x10, 0xf0,
    0xf0, 0x90, 0xf0, 0x90, 0x90,
    0xe0, 0x90, 0xf0, 0x10, 0xf0,
    0xf0, 0x80, 0x80, 0x80, 0xf0,
    0xe0, 0x90, 0x90, 0x90, 0xe0,
    0xf0, 0x80, 0xf0, 0x80, 0xf0,
    0xf0, 0x80, 0xf0, 0x80, 0x80
};

s_interpreter* init_interpreter() {
    s_interpreter* _interpreter = malloc(sizeof(s_interpreter));
    memset(_interpreter, 0x00, sizeof(s_interpreter));

    // initialize digits portion of memory
    for (int i = 0; i < sizeof(digits); i++) {
        _interpreter->mem[i] = digits[i];
    }

    _interpreter->pc = 0x200;
    _interpreter->sp = 0x1ff; // downward facing stack

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

    // self check (skip digits)
    for (int i = 5 * 16; i < 0x200; i++) {
        if (interpreter->mem[i] != 0) {
            printf("Invalid starting value at memory location %x: %x\n", i, interpreter->mem[i]);
            exit(1);
        }
    }

    printf("Read %i bytes\n", fsize);
}

void push(s_interpreter* interpreter) {
    // push lo, push hi
    interpreter->mem[interpreter->sp--] = interpreter->pc & 0xffu;
    interpreter->mem[interpreter->sp--] = interpreter->pc >> 8u;
}

void pop(s_interpreter* interpreter) {
    // pop hi, pop lo
    interpreter->pc = interpreter->mem[++interpreter->sp] << 8u;
    interpreter->pc |= interpreter->mem[++interpreter->sp];
}

unsigned char wait_for_keypress(s_interpreter* interpreter) {
    while (true) {
        SDL_Event e;

        while (SDL_PollEvent( &e ) != 0) {
            if (e.type == SDL_KEYDOWN) {
                for (unsigned char i = 0; i < 16; i++) {
                    if (keys[i] == SDL_GetKeyFromScancode(e.key.keysym.scancode)) {
                        interpreter->keyboard[i] = true;
                        return i;
                    }
                }
            }
        }
    }
}

bool DRW(s_interpreter* interpreter, unsigned int length, unsigned char x, unsigned char y) {

    bool colission = false;
    int index;
    for (unsigned char i = 0; i < (unsigned char)length; i++) {
        for (unsigned char bit_count = 0; bit_count < 8; bit_count++) {
            index = WIDTH * ((unsigned)(y + i) & (HEIGHT - 1u)) + ((unsigned)(x + bit_count) & (WIDTH - 1u));
            colission |=
                    (unsigned)(interpreter->display[index] && interpreter->mem[interpreter->I + i] & (0x80u >> bit_count));

            interpreter->display[index] ^=
                     (interpreter->mem[interpreter->I + i] & (0x80u >> bit_count)) ? ON_COLOR : OFF_COLOR;
        }
    }
    return colission;
}

void BCD(s_interpreter* interpreter, unsigned char number) {
    char dec[4];        // NULL terminator
    sprintf(dec, "%03d", number);

    for (int i = 0; i < 3; i++) {
        interpreter->mem[interpreter->I + i] = dec[i] - '0';
    }
}

void step(s_interpreter* interpreter) {
    unsigned short instruction = (unsigned)(interpreter->mem[interpreter->pc++] << 8u) | interpreter->mem[interpreter->pc++];

    char x = (char)((instruction & 0x0f00u) >> 8u);
    char y = (char)((instruction & 0x00f0u) >> 4u);

    switch ((unsigned)(instruction >> 12u) & 0xfu){
        case 0x0u:
            if (instruction == 0x00e0) {
                // CLS
                memset(interpreter->display, 0x00, sizeof(interpreter->display));
            }
            else if (instruction == 0x00ee) {
                // RET
                pop(interpreter);
            }
            break;
        case 0x1u:
            // JP
            interpreter->pc = (instruction & 0x0fffu);
            break;
        case 0x2u:
            // CALL
            push(interpreter);
            interpreter->pc = (instruction & 0x0fffu);
            break;
        case 0x3u:
            // SE
            if (interpreter->registers[x] == (instruction & 0x00ffu)) {
                interpreter->pc += 2;
            }
            break;
        case 0x4u:
            // SNE
            if (interpreter->registers[x] != (instruction & 0x00ffu)) {
                interpreter->pc += 2;
            }
            break;
        case 0x5u:
            // SE
            if (interpreter->registers[x] == interpreter->registers[y]) {
                interpreter->pc += 2;
            }
            break;
        case 0x6u:
            // LD
            interpreter->registers[x] = (instruction & 0xffu);
            break;
        case 0x7u:
            // ADD
            interpreter->registers[x] += (unsigned char)(instruction & 0xffu);
            break;
        case 0x8u:
            switch (instruction & 0x000fu) {
                case 0x0u:
                    // LD
                    interpreter->registers[x] = interpreter->registers[y];
                    break;
                case 0x1u:
                    // OR
                    interpreter->registers[x] |= interpreter->registers[y];
                    break;
                case 0x2u:
                    // AND
                    interpreter->registers[x] &= interpreter->registers[y];
                    break;
                case 0x3u:
                    // XOR
                    interpreter->registers[x] ^= interpreter->registers[y];
                    break;
                case 0x4u:
                    // ADD
                    interpreter->registers[0xf] = ((int)interpreter->registers[x] + (int)interpreter->registers[y]) > 0xffu ? 1 : 0;
                    interpreter->registers[x] += interpreter->registers[y];
                    break;
                case 0x5u:
                    // SUB
                    interpreter->registers[0xf] = (interpreter->registers[x] > (int)interpreter->registers[y]) ? 1 : 0;
                    interpreter->registers[x] -= interpreter->registers[y];
                    break;
                case 0x6u:
                    // SHR
                    interpreter->registers[0xf] = interpreter->registers[x] & 1u;
                    interpreter->registers[x] >>= 1u;
                    break;
                case 0x7u:
                    // SUBN
                    interpreter->registers[0xf] = (interpreter->registers[x] < (int)interpreter->registers[y]) ? 1 : 0;
                    interpreter->registers[x] = interpreter->registers[y] - interpreter->registers[x];
                    break;
                case 0xeu:
                    // SHL
                    interpreter->registers[0xf] = (interpreter->registers[x] & 0x80u) ? 1 : 0;
                    interpreter->registers[x] <<= 1u;
                    break;
                default:
                    printf("Invalid instruction: %x\n", instruction);
                    exit(1);
            }
            break;
        case 0x9u:
            // SNE
            if (interpreter->registers[x] != interpreter->registers[y]) {
                interpreter->pc += 2;
            }
            break;
        case 0xau:
            // LD I
            interpreter->I = instruction & 0x0fffu;
            break;
        case 0xbu:
            // JP V0
            interpreter->pc = interpreter->registers[0] + (instruction & 0x0fffu);
            break;
        case 0xcu:
            // RND
            interpreter->registers[x] = rand() & instruction & 0xffu;
            break;
        case 0xdu:
            interpreter->registers[0xf] =
                    DRW(interpreter, instruction & 0x000fu, interpreter->registers[x], interpreter->registers[y]) ? 1 : 0;
            break;
        case 0xeu:

            if ((instruction & 0x00ffu) == 0x009eu) {
                // SKP
                if (interpreter->keyboard[interpreter->registers[x]]) {
                    interpreter->pc += 2;
                }
            }
            else if ((instruction & 0x00ffu) == 0x00a1u) {
                // SKNP
                if (!interpreter->keyboard[interpreter->registers[x]]) {
                    interpreter->pc += 2;
                }
            }
            else {
                printf("Invalid instruction: %x\n", instruction);
                exit(1);
            }
            break;
        case 0xfu:
            switch (instruction & 0x00ffu) {
                case 0x07u:
                    // LD
                    interpreter->registers[x] = interpreter->dt;
                    break;
                case 0x0au:
                    // LD
                    interpreter->registers[x] = wait_for_keypress(interpreter);
                    break;
                case 0x15u:
                    // LD DT
                    interpreter->dt = interpreter->registers[x];
                    break;
                case 0x18u:
                    // LD ST
                    interpreter->st = interpreter->registers[x];
                    break;
                case 0x1eu:
                    // ADD I
                    interpreter->I += interpreter->registers[x];
                    break;
                case 0x29u:
                    // LD F
                    // 5 bytes per digit, starting at 0
                    interpreter->I = 5 * interpreter->registers[x];
                    break;
                case 0x33u:
                    // BCD
                    BCD(interpreter, interpreter->registers[x]);
                    break;
                case 0x55u:
                    // LD [I]
                    for (int i = 0; i <= x; i++) {
                        interpreter->mem[interpreter->I + i] = interpreter->registers[i];
                    }
                    break;
                case 0x65u:
                    // LD
                    for (int i = 0; i <= x; i++) {
                        interpreter->registers[i] = interpreter->mem[interpreter->I + i];
                    }
                    break;
                default:
                    printf("Invalid instruction: %x\n", instruction);
                    exit(1);
            }
            break;
        default:
            printf("Invalid instruction code: %x\n", instruction);
            exit(1);
    }

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
                    for (int i = 0; i < 16; i++) {
                        if (keys[i] == SDL_GetKeyFromScancode(e.key.keysym.scancode)) {
                            interpreter->keyboard[i] = true;
                            break;
                        }
                    }
                    break;
                case SDL_KEYUP:
                    for (int i = 0; i < 16; i++) {
                        if (keys[i] == SDL_GetKeyFromScancode(e.key.keysym.scancode)) {
                            interpreter->keyboard[i] = false;
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        for (int i = 0; i < INSTRS_PER_FRAME; i++) {
            step(interpreter);
        }

        if (interpreter->dt) {
            interpreter->dt--;
        }

        if (interpreter->st) {
            interpreter->st--;
        }

        blit_bitmap_32bppRGBA(interpreter->display, display_rect);
#ifdef LIMIT_FRAMERATE
        nanosleep(&frame_delay, NULL);
#endif
    }

    close_display();
    return 0;
}
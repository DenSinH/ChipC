#ifndef CHIP8_INTERPRETER_H
#define CHIP8_INTERPRETER_H

#include <stdbool.h>

#define WIDTH 64
#define HEIGHT 32
#define SCALE 4

#define SCREEN_WIDTH (WIDTH * SCALE)
#define SCREEN_HEIGHT (HEIGHT * SCALE)

typedef struct {
    unsigned char registers[16];
    unsigned short I;
    unsigned char dt, st;
    unsigned short pc;

    unsigned short sp;  // we put the stack in the lower 0x200 bytes of the memory (should be enough I reckon)

    unsigned char mem[0x1000];
    unsigned int display[WIDTH * HEIGHT];
    bool keyboard[16];
} s_interpreter;

s_interpreter* init_interpreter();
int run(s_interpreter* Interpreter);

#endif //CHIP8_INTERPRETER_H

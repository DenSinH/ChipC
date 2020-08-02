#ifndef CHIP8_INTERPRETER_H
#define CHIP8_INTERPRETER_H

#define WIDTH 64
#define HEIGHT 32
#define SCALE 4

#define SCREEN_WIDTH (WIDTH * SCALE)
#define SCREEN_HEIGHT (HEIGHT * SCALE)

typedef struct {
    unsigned int registers[16];
    unsigned short int I;
    unsigned short int dt, st;
    unsigned short int pc;

    unsigned char mem[0x1000];
    unsigned int display[WIDTH * HEIGHT];
} s_interpreter;


s_interpreter* init_interpreter();
int run(s_interpreter* Interpreter);

#endif //CHIP8_INTERPRETER_H

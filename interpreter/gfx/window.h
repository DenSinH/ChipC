#ifndef HELLOWORLD_WINDOW_H
#define HELLOWORLD_WINDOW_H

#include <stdbool.h>

bool init_display(int width, int height);
void close_display();
bool blit_bitmap_32bppRGBA();

#endif //HELLOWORLD_WINDOW_H

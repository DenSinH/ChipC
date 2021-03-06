#include <stdio.h>
#include <SDL2/SDL.h>

#include "window.h"

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

SDL_Rect _screen_rect;

bool init_display(int width, int height)
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow( "Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );

            // initialize screen rectangle (target rectangle)
            _screen_rect.x = 0;
            _screen_rect.y = 0;
            _screen_rect.w = width;
            _screen_rect.h = height;
        }
    }

    return success;
}

void close_display()
{
    //Destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

bool blit_bitmap_32bppRGBA(unsigned int raw[], SDL_Rect src) {
    SDL_Surface* gBitmap = SDL_CreateRGBSurfaceFrom(
            raw,
            src.w,
            src.h,
            32,
            src.w * 4,
            0x00ff0000,
            0x0000ff00,
            0x000000ff,
            0
    );

    //Load media
    if( gBitmap == NULL )
    {
        printf( "Failed to load media: %s\n", SDL_GetError() );
        return false;
    }
    else
    {
        //Apply the image
        SDL_BlitScaled( gBitmap, &src, gScreenSurface, &_screen_rect );

        //Update the surface
        SDL_UpdateWindowSurface( gWindow );

        SDL_FreeSurface(gBitmap);

        return true;
    }
}

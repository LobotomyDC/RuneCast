/* 

    SDL_framerate - test program for framerate manager 

    Copyright (C) A. Schiffler, August 2001

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#ifdef DREAMCAST
#include <kos.h>
KOS_INIT_FLAGS(INIT_DEFAULT);
#endif

#include "include/SDL.h"
#include "include/SDL_framerate.h"
#include "include/SDL_gfxPrimitives.h"

#define SCR_X 320
#define SCR_Y 240

void ClearEvents()
{
    SDL_Event event;
    SDL_Delay(333);
    while(SDL_PollEvent(&event))
        SDL_Delay(50);
}

void HandleEvent()
{
    SDL_Event event; 
    /* Check for events */
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYBUTTONDOWN:
            case SDL_KEYDOWN:
            case SDL_QUIT:
                exit(0);
                break;
        }
    }
}

void Draw(SDL_Surface *screen)
{
    int i, rate, x, y, dx, dy, r, g, b;
    FPSmanager fpsm;
 
    /* Initialize variables */
    srand(SDL_GetTicks());
    i = 0;
    x = SCR_X / 2;
    y = SCR_Y / 2;
    dx = 7;
    dy = 5;
    r = g = b = 255;
  
    ClearEvents();
    SDL_initFramerate(&fpsm);
    while (1) {
        /* Set/switch framerate */
        i -= 1;
        if (i < 0) {
            /* Set new rate */
            rate = 5 + 5 * (rand() % 10);
            SDL_setFramerate(&fpsm, rate);
            /* New timeout */
            i = 2 * rate;
            /* New Color */
            r = rand() & 255;
            g = rand() & 255;
            b = rand() & 255;
        }
        HandleEvent();
        SDL_FillRect(screen, NULL, 0);
        /* Move */
        x += dx;
        y += dy;
  
        /* Reflect */
        if ((x < 0) || (x > SCR_X)) { dx = -dx; }
        if ((y < 0) || (y > SCR_Y)) { dy = -dy; }
        /* Draw */
        filledCircleRGBA(screen, x, y, 30, r, g, b, 255);
        circleRGBA(screen, x, y, 30, 255, 255, 255, 255);
    
        SDL_Flip(screen);
        /* Delay to fix rate */                   
        SDL_framerateDelay(&fpsm);  
    }
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen;
    Uint32 video_flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
    Uint32 video_bits = 0;

    #if defined(DREAMCAST) || defined(GP32)
    video_flags |= SDL_FULLSCREEN;
    #ifdef GP32
    x_gp32_SetCPUSpeed_133();
    video_bits = 8;
    #else
    video_bits = 16;
    #endif
    #endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    #ifdef DREAMCAST
    puts("\n--------------\n| DMA DRIVER |\n--------------");
    SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
    #endif

    screen = SDL_SetVideoMode(SCR_X, SCR_Y, video_bits, video_flags);

    if (screen == NULL) {
        fprintf(stderr, "Couldn't set %dx%dx%lu video mode: %s\n",
                SCR_X, SCR_Y, (unsigned long)video_bits, SDL_GetError());
        exit(1);
    }

    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);

    Draw(screen);

    return 0;
}
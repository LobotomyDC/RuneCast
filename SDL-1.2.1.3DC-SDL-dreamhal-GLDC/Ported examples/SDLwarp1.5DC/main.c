/*-----------------------------------------------------------------------
    Warp-SDL 1.1

    Simple DirectMedia Layer demo
    Realtime picture 'gooing'

    main.c: main program

    written by Emmanuel Marty <core@mirus.fr>
    1.0: November 1st, 1997
        initial version
    1.1: November 9th, 1997
        support for any resolution
        support for 8,15,16,24,32 bpp
        color depth conversion
        usage of GGI datatypes
    1.1.3: November 12th, 1997
        moved color depth conversion code to color.c

    modified for SDL by Sam Lantinga <slouken@devolution.com>
    1.1.4: April 13th, 1998
        initial modification
        
    Ported to Dreamcast by Ian Micheal 
    10/10/2024
    SDL1.2.1.3DC dreamhal

    Updated with multiple images and D-pad warp control

    Released under GNU Public License
-----------------------------------------------------------------------*/

#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/SDL.h"    
#include "include/SDL_image.h"
#include "include/SDL_dreamcast.h" 

#include <math.h>
#include "warp.h"

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define NUM_IMAGES 3
#define WARP_INTENSITY_STEP 0.1f

KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

extern struct warp *initWarp(SDL_Surface *src);
extern void disposeWarp(struct warp *w);
extern void doWarp8bpp(struct warp *w, int xw, int yw, int cw);
extern void doWarp16bpp(struct warp *w, int xw, int yw, int cw);
extern void doWarp24bpp(struct warp *w, int xw, int yw, int cw);
extern void doWarp32bpp(struct warp *w, int xw, int yw, int cw);

float fclampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void clearevents(void) {
    SDL_Event event;
    SDL_Delay(100);
    while(SDL_PollEvent(&event))
        SDL_Delay(20);
}

int HandleEvent(int *current_image, float *warp_intensity) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.button == 13) {  // Start button
                    printf("Start Button pressed - Exiting\n");
                    return 1;  // Exit the program
                } else if (event.jbutton.button == 2) {  // A button
                    *current_image = (*current_image + 1) % NUM_IMAGES;
                    printf("Switched to image %d\n", *current_image + 1);
                }
                break;
            case SDL_JOYHATMOTION:
                if (event.jhat.value & SDL_HAT_UP) {
                    *warp_intensity = fclampf(*warp_intensity + WARP_INTENSITY_STEP, 0.1f, 2.0f);
                    printf("Warp intensity: %.2f\n", *warp_intensity);
                } else if (event.jhat.value & SDL_HAT_DOWN) {
                    *warp_intensity = fclampf(*warp_intensity - WARP_INTENSITY_STEP, 0.1f, 2.0f);
                    printf("Warp intensity: %.2f\n", *warp_intensity);
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return 1;
                break;
            case SDL_QUIT:
                return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    SDL_Surface *screen;
    SDL_Surface *pictures[NUM_IMAGES], *convert;
    struct warp *w;
    int xw, yw, cw, tval;
    void (*doWarp)(struct warp *w, int xw, int yw, int cw);
    SDL_Joystick *joystick;
    float warp_intensity = 1.0f;
    int current_image = 0;

    printf("Initializing...\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_DC_ShowAskHz(SDL_FALSE);
    SDL_DC_Default60Hz(SDL_FALSE);
    SDL_DC_VerticalWait(SDL_FALSE);

    SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);
    
    printf("Setting up video mode...\n");
    SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
    screen = SDL_SetVideoMode(320, 200, 16, SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
        return 1;
    }

    printf("Loading images...\n");
    const char* image_files[] = {"/rd/image1.bmp", "/rd/image2.bmp", "/rd/image3.bmp"};
    for (int i = 0; i < NUM_IMAGES; i++) {
        SDL_Surface* picture = IMG_Load(image_files[i]);
        if (picture == NULL) {
            fprintf(stderr, "Couldn't load %s: %s\n", image_files[i], SDL_GetError());
            return 1;
        }

        /* Seed the palette, don't worry if it doesn't take */
        if (picture->format->palette) {
            SDL_SetColors(screen, picture->format->palette->colors, 0,
                          picture->format->palette->ncolors);
        }

        /* Convert the picture to the display format for speed */
        convert = SDL_ConvertSurface(picture, screen->format, SDL_SWSURFACE);
        SDL_FreeSurface(picture);
        if (convert == NULL) {
            fprintf(stderr, "Couldn't convert image: %s\n", SDL_GetError());
            return 1;
        }
        pictures[i] = convert;
    }
    
    SDL_ShowCursor(SDL_DISABLE);
    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);
    if (!joystick) {
        fprintf(stderr, "Failed to open joystick!\n");
    } else {
        printf("Joystick opened successfully\n");
    }

    printf("Initializing warp structure...\n");
    w = initWarp(pictures[0]);
    if (w == NULL) {
        fprintf(stderr, "Couldn't initialize warp structure\n");
        for (int i = 0; i < NUM_IMAGES; i++) {
            SDL_FreeSurface(pictures[i]);
        }
        return 1;
    }
    w->dst = screen;

    switch (screen->format->BytesPerPixel) {
        case 1: doWarp = doWarp8bpp; break;
        case 2: doWarp = doWarp16bpp; break;
        case 3: doWarp = doWarp24bpp; break;
        case 4: doWarp = doWarp32bpp; break;
        default:
            fprintf(stderr, "Unknown BytesPerPixel: %d\n", screen->format->BytesPerPixel);
            return 1;
    }

    clearevents();

    printf("Entering main loop...\n");
    tval = 0;
    int quit = 0;
    int previous_image = 0;
    while (!quit) {
        if (HandleEvent(&current_image, &warp_intensity)) {
            quit = 1;
            continue;
        }

        // Check if the image has changed
        if (current_image != previous_image) {
            // Dispose of the old warp structure
            disposeWarp(w);

            // Initialize a new warp structure with the new image
            w = initWarp(pictures[current_image]);
            if (w == NULL) {
                fprintf(stderr, "Couldn't initialize warp structure for image %d\n", current_image);
                quit = 1;
                continue;
            }
            w->dst = screen;

            previous_image = current_image;
            printf("Switched to image %d\n", current_image + 1);
        }

        xw = (int)(sin((tval+100)*M_PI/128) * 30 * warp_intensity);
        yw = (int)(sin((tval)*M_PI/256) * -35 * warp_intensity);
        cw = (int)(sin((tval-70)*M_PI/64) * 50 * warp_intensity);
        xw += (int)(sin((tval-10)*M_PI/512) * 40 * warp_intensity);
        yw += (int)(sin((tval+30)*M_PI/512) * 40 * warp_intensity);

        if (SDL_LockSurface(screen) < 0) continue;

        w->framebuf = (Uint8 *)screen->pixels;
        doWarp(w, xw, yw, cw);

        SDL_UnlockSurface(screen);
        SDL_Flip(screen);

        tval = (tval+1) & 511;
    }

    printf("Cleaning up...\n");
    disposeWarp(w);
    for (int i = 0; i < NUM_IMAGES; i++) {
        SDL_FreeSurface(pictures[i]);
    }
    if (joystick) {
        SDL_JoystickClose(joystick);
    }
    SDL_Quit();

    printf("Program ended.\n");
    return 0;
}
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

#ifndef WARP_H
#define WARP_H

#include "include/SDL.h"    
#include "include/SDL_image.h"
#include "include/SDL_dreamcast.h" 

struct warp {
    SDL_Surface *src;
    SDL_Surface *dst;
    Uint8 *framebuf;
    void **offstable;
    Sint32 *disttable;
    Sint32 sintable[1280];
    Sint32 ctable[1024];
};

#endif /* WARP_H */

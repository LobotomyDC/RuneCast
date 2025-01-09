/* 

    TestFonts - test dynamic font loading code

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

#ifdef GP32
#include <x_gp32.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "include/SDL.h"

#include "include/SDL_gfxPrimitives.h"

#include "include/SDL_gfx_5x7_fnt.h"
#include "include/SDL_gfx_7x13_fnt.h"
#include "include/SDL_gfx_7x13B_fnt.h"
#include "include/SDL_gfx_7x13O_fnt.h"
#include "include/SDL_gfx_9x18_fnt.h"
#include "include/SDL_gfx_9x18B_fnt.h"


#include "include/SDL_gfx_fnt.h"

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
        while ( SDL_PollEvent(&event) ) {
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
 char mytext[] = "abcdefghijklmnopqrstuvwxyz";
 char mytext2[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
 
 SDL_FillRect(screen,NULL,0);

 gfxPrimitivesSetFont(&SDL_gfx_font_5x7_fnt,5,7);
 stringRGBA(screen,10,00,mytext,255,255,255,255);
 stringRGBA(screen,10,20,mytext2,255,255,255,255);

 gfxPrimitivesSetFont(&SDL_gfx_font_7x13_fnt,7,13);
 stringRGBA(screen,10,40,mytext,255,255,255,255);
 stringRGBA(screen,10,60,mytext2,255,255,255,255);

 gfxPrimitivesSetFont(&SDL_gfx_font_7x13B_fnt,7,13);
 stringRGBA(screen,10,80,mytext,255,255,255,255);
 stringRGBA(screen,10,100,mytext2,255,255,255,255);

 gfxPrimitivesSetFont(&SDL_gfx_font_7x13O_fnt,7,13);
 stringRGBA(screen,10,120,mytext,255,255,255,255);
 stringRGBA(screen,10,140,mytext2,255,255,255,255);
 
 gfxPrimitivesSetFont(&SDL_gfx_font_9x18_fnt,9,18);
 stringRGBA(screen,10,160,mytext,255,255,255,255);
 stringRGBA(screen,10,180,mytext2,255,255,255,255);
 
 gfxPrimitivesSetFont(&SDL_gfx_font_9x18B_fnt,9,18);
 stringRGBA(screen,10,200,mytext,255,255,255,255);
 stringRGBA(screen,10,220,mytext2,255,255,255,255);
 
 SDL_Flip(screen);

 ClearEvents();
 while (1) {
  HandleEvent();
  SDL_Delay(100);
 }
}

int main ( int argc, char *argv[] )
{
	SDL_Surface *screen;
	Uint32 video_flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
	Uint32 video_bits=0;

#if defined(DREAMCAST) || defined(GP32)
	video_flags |= SDL_FULLSCREEN;
#ifdef GP32
	x_gp32_SetCPUSpeed_133();
	video_bits=8;
#else
	video_bits=16;
#endif
#endif

	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0 ) {
		fprintf(stderr,
			"Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	screen = SDL_SetVideoMode(320, 240, video_bits, video_flags);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
					320, 240, video_bits, SDL_GetError());
		exit(1);
	}

	SDL_JoystickEventState(SDL_ENABLE);
	SDL_JoystickOpen(0);

	Draw (screen);

	return(0);
}

/* 

    SDL_rotozoom - test program 

    Copyright (C) A. Schiffler, July 2001

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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/SDL.h"

#include "include/SDL_rotozoom.h"

#include "sample8.h"

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

#define ClearScreen(SCREEN) SDL_FillRect(SCREEN,NULL,0);


void RotatePicture (SDL_Surface *screen, SDL_Surface *picture, int rotate, int flip, int smooth) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	sll angle, zoomf, zoomfx, zoomfy;

	/* Rotate and display the picture */
	framemax=4*360; frameinc=1;
	for (framecount=360; framecount<framemax; framecount += frameinc) {
		if ((framecount % 360)==0) frameinc++;
		HandleEvent();
		ClearScreen(screen);
                zoomf=slldiv(int2sll(framecount),int2sll(framemax));
                zoomf=sllmul(sllmul(dbl2sll(1.5),zoomf),zoomf);
                /* Are we in flipping mode? */
		if (flip) {
		 /* Flip X factor */
                 if (flip & 1) {
                  zoomfx=-zoomf;
                 } else {
                  zoomfx=zoomf;
                 }
                 /* Flip Y factor */
                 if (flip & 2) {
                  zoomfy=-zoomf;
                 } else {
                  zoomfy=zoomf;
                 }
                 angle=int2sll(framecount*rotate);
		 if ((rotozoom_picture=rotozoomSurfaceXY_sll (picture, angle, zoomfx, zoomfy, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				break;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }
		} else {
                 angle=int2sll(framecount*rotate);
		 if ((rotozoom_picture=rotozoomSurface_sll (picture, angle, zoomf, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				break;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }
                }
		/* Display by flipping screens */
		SDL_Flip(screen);
	}
	
	if (rotate) {
		/* Final display with angle=0 */
		HandleEvent();
		ClearScreen(screen);
		if (flip) {
 		 if ((rotozoom_picture=rotozoomSurfaceXY_sll (picture, dbl2sll(0.01), zoomfx, zoomfy, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				return;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }		
		} else {
 		 if ((rotozoom_picture=rotozoomSurface_sll (picture, dbl2sll(0.01), zoomf, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				return;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }		
	        }
		/* Display by flipping screens */
		SDL_Flip(screen);
	}

	/* Pause for a sec */
	SDL_Delay(1000);
}

void ZoomPicture (SDL_Surface *screen, SDL_Surface *picture, int smooth) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	sll zoomxf,zoomyf;

	/* Zoom and display the picture */
	framemax=4*360; frameinc=1;
	for (framecount=360; framecount<framemax; framecount += frameinc) {
		if ((framecount % 360)==0) frameinc++;
		HandleEvent();
		ClearScreen(screen);
                zoomxf=slldiv(int2sll(framecount),int2sll(framemax));
                zoomxf=sllmul(sllmul(dbl2sll(1.5),zoomxf),zoomxf);
                zoomyf=slladd(dbl2sll(0.5),sllabs(sllsin(slldiv(int2sll(framecount),dbl2sll(80.0)))));
		if ((rotozoom_picture=zoomSurface_sll (picture, zoomxf, zoomyf, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				break;
			}
			SDL_FreeSurface(rotozoom_picture);
		}

		/* Display by flipping screens */
		SDL_Flip(screen);
	}
	
	/* Pause for a sec */
	SDL_Delay(1000);
}

#define ROTATE_OFF	0
#define ROTATE_ON	1

#define FLIP_OFF	0
#define FLIP_X		1
#define FLIP_Y		2
#define FLIP_XY		3

void Draw (SDL_Surface *screen)
{
	SDL_Surface *picture= SDL_LoadBMP_RW(SDL_RWFromMem(&sample8_bmp,SAMPLE8_LEN),0);
	if (screen->format->BytesPerPixel<2)
		SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, picture->format->palette->colors,0,256);
	else
	{
		SDL_Surface *tmp = picture;
		picture=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
	}
	while(1)
	{
	   RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_OFF);
	   RotatePicture(screen,picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_OFF);
	   ZoomPicture(screen,picture,SMOOTHING_OFF);
	   RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_ON);
	   RotatePicture(screen,picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON);
	   ZoomPicture(screen,picture,SMOOTHING_ON);
	}
	SDL_FreeSurface(picture);
}

int main ( int argc, char *argv[] )
{
	SDL_Surface *screen;
	Uint32 video_flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
	Uint32 video_bits = 0;

#if defined(DREAMCAST) || defined(GP32)
	video_flags |= SDL_FULLSCREEN;
	video_bits=8;
#ifdef GP32
	x_gp32_SetCPUSpeed_133();
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

	{
		SDL_Event event;
		SDL_Delay(333);
		while(SDL_PollEvent(&event))
			SDL_Delay(50);
	}

	Draw (screen);
	
	return(0);
}

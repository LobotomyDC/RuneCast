
/*
	Name: Ian micheal
	Copyright: 2021
	Author: SDL example sorry i dont know / ported and refined By Ian micheal
	Date: 19/06/21 13:21
	Description: SDL dreamhal 1.0 video benchamrk 
*/


#include<kos.h>
#include<stdio.h>
#include "SDLDH/SDL.h"
#include "SDLDH/SDL_dreamcast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Screen coords */
#define SCR_X 640
#define SCR_Y 480
int VIDEOX;
int VIDEOY;


static unsigned char spriteData[] = {
0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,
0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,
0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,1,0,0,0,0,
0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


static int getInfo(SDL_Surface *surface)
{
	printf("Pitch = %d\n", surface->pitch);

	printf("Hardware surfaces avail  = %d\n", SDL_GetVideoInfo()->hw_available);
	printf("Window manager avail     = %d\n", SDL_GetVideoInfo()->wm_available);
	printf("Blitter hardware         = %d\n", SDL_GetVideoInfo()->blit_hw);
	printf("Colorkey blit hardware   = %d\n", SDL_GetVideoInfo()->blit_hw_CC);
	printf("Alpha blit hardware      = %d\n", SDL_GetVideoInfo()->blit_hw_A);
	printf("Software->Hardware accel = %d\n", SDL_GetVideoInfo()->blit_sw);
	if (SDL_GetVideoInfo()->hw_available == 1)
		printf("Video memory             = %d\n", SDL_GetVideoInfo()->video_mem);
	else
		printf("Video memory             = N/A\n");

	return SDL_GetVideoInfo()->hw_available;
}

static int doSlowPoints(SDL_Surface *surface)
{
	int numFrames = 8;
	int k, i, j, ofs, yofs;

	if (SDL_LockSurface(surface) <0) return k;
	for (k=0; k<numFrames; k++)
	{
		yofs = 0;
		for (i=0; i<VIDEOY; i++)
		{
			for (j=0, ofs=yofs; j<VIDEOX; j++, ofs++)
			{
				//fprintf(stderr, "k=%d i=%d j=%d\n", k, i, j);
				((unsigned char *)surface->pixels)[ofs] = (unsigned char)255 -k*16 -k;
			}
			yofs += surface->pitch;
			SDL_UpdateRect(surface, 0, 0, VIDEOX, VIDEOY);
		}
	}
	SDL_UnlockSurface(surface);
	return k;
}


static int doFastPoints(SDL_Surface *surface)
{
	int numFrames = 256;
	int k, i, j, ofs, yofs;

	if (SDL_LockSurface(surface) <0) return k;
	for (k=0; k<numFrames; k++)
	{
		yofs = 0;
		for (i=0; i<VIDEOY; i++)
		{
			for (j=0, ofs=yofs; j<VIDEOX; j++, ofs++)
			{
				//fprintf(stderr, "k=%d i=%d j=%d\n", k, i, j);
				((unsigned char *)surface->pixels)[ofs] = (unsigned char)255 -k;
			}
			yofs += surface->pitch;
		}
		SDL_UpdateRect(surface, 0, 0, VIDEOX, VIDEOY);
	}
	SDL_UnlockSurface(surface);
	return k;
}

static int doRects(SDL_Surface *surface)
{
	int numRects = 4096;

	int k;
	SDL_Rect rect;

	if (SDL_LockSurface(surface) <0) return k;
	rect.x = 0;
	rect.y = 0;
	for (k=0; k<numRects; k++)
	{
		rect.x++;
		if (rect.x == VIDEOX -6) rect.x = 0;
		rect.y++;
		if (rect.y == VIDEOY -6) rect.y = 0;
		rect.w = (VIDEOX -rect.x)/2;
		rect.h = (VIDEOY -rect.y)/2;
		
		SDL_FillRect(surface, &rect, k % 256);
		SDL_UpdateRect(surface, rect.x, rect.y, rect.w, rect.h);
	}
	SDL_UnlockSurface(surface);
	return k;
}


static int doBlitter(SDL_Surface *surface)
{
	int numBlits = 4096;

	int i,j;
	int ofs;
	int minx, miny, maxx, maxy;
	SDL_Rect rect, orect;
	SDL_Color colors[3];
	colors[0].r = 0;
	colors[0].g = 0;
	colors[0].b = 0;
	colors[1].r = 255;
	colors[1].g = 255;
	colors[1].b = 0;
	colors[2].r = 255;
	colors[2].g = 0;
	colors[2].b = 0;
	SDL_SetColors(surface, colors, 0, 3);

	if (SDL_LockSurface(surface) <0) return 0;
	rect.x = 0;
	rect.y = 0;
	rect.w = VIDEOX;
	rect.h = VIDEOY;
	SDL_FillRect(surface, &rect, 0);
	for (i=0; i<32; i++)
		for (j=0; j<32; j++)
		{
			ofs = j*surface->pitch+i;
			//((unsigned char *)surface->pixels)[ofs] = (unsigned char)(rand()*255/RAND_MAX);
			((unsigned char *)surface->pixels)[ofs] = spriteData[j*32+i];
		}
	SDL_UpdateRect(surface, 0, 0, 32, 32);
	SDL_UnlockSurface(surface);

	rect.x = 32;
	rect.y = 32;
	rect.w = 32;
	rect.h = 32;
	orect.x = 0;
	orect.y = 0;
	orect.w = 32;
	orect.h = 32;

	rect.x = 0;
	rect.y = 0;
	for (i=0; i<numBlits; i++)
	{
		orect.x = rect.x;
		orect.y = rect.y;
		rect.x++;
		if (rect.x >= VIDEOX -32) rect.x=0;
		rect.y++;
		if (rect.y >= VIDEOY -32) rect.y=0;
		minx = (orect.x < rect.x) ? orect.x : rect.x;
		miny = (orect.y < rect.y) ? orect.y : rect.y;
		SDL_BlitSurface(surface, &orect, surface, &rect);
		SDL_UpdateRect(surface, minx, miny, 32, 32);
	}
	return i;
}

int main (int argc, char **argv)
{
  Uint32 video_flags;
  SDL_Surface *screen;
  unsigned char *t;
  time_t starttime;
  int i, iter, state=0;
  int hw_avail=1;
  double results[4][4];

SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK| SDL_INIT_TIMER);
    puts("\n-----------------\n| DIRECT DRIVER |\n-----------------");
 // SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
   SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
  for (iter=0; iter<4; iter++)
  {
    for (i=0; i<4; i++)
      results[iter][i] = -1.0;

    switch (iter)
    { 	puts("\n-----------------\n| DIRECT DRIVER |\n-----------------");
	SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
      case 0:  VIDEOX = 320;  VIDEOY = 240;
               video_flags = (SDL_FULLSCREEN|SDL_HWSURFACE);
               fprintf(stderr, "Mode = 320x240, software SDL_HWSURFACE|SDL_FULLSCREEN normal\n");
               break;
               	puts("\n--------------\n| DMA DRIVER |\n--------------");
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
      case 1:  VIDEOX = 320;  VIDEOY = 240;
               video_flags = (SDL_FULLSCREEN|SDL_DOUBLEBUF|SDL_HWSURFACE);
               fprintf(stderr, "Mode = 320x240, hardware SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN \n");
            
               break;
	puts("\n-------------------\n| TEXTURED DRIVER |\n-------------------");
	SDL_DC_SetVideoDriver(SDL_DC_TEXTURED_VIDEO);
      case 2:  VIDEOX = 512;  VIDEOY = 256;
        SDL_DC_SetWindow(SCR_X,SCR_Y);
               video_flags = (SDL_HWSURFACE);
               	
               fprintf(stderr, "Mode = 640x480, software SDL_HWSURFACE normal\n");
               break;
               	puts("\n--------------\n| DMA DRIVER |\n--------------");
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
      case 3:  VIDEOX = 640;  VIDEOY = 480;
               video_flags = (SDL_FULLSCREEN|SDL_DOUBLEBUF|SDL_HWSURFACE);
               fprintf(stderr, "Mode = 640x480, hardware SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN \n");
            
               break;
    }

    screen=SDL_SetVideoMode(VIDEOX, VIDEOY, 8, video_flags);
    if ( screen == NULL ) {
      fprintf(stderr, "Unable to set video mode.\n");
      continue;
    }

    if (iter == 0)
    {
      SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
      SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

      starttime = 1;
      state = 0;

      hw_avail = getInfo(screen);
      printf("\n");
    }

    // Slow points test
    //
    starttime=SDL_GetTicks(); 
    fprintf(stderr, "Slow points test\n");
    state = doSlowPoints(screen);
    results[iter][0] = ((double)state*1000)/(SDL_GetTicks() -starttime);
   
    // Fast points test
    // 
    starttime=SDL_GetTicks(); 
    fprintf(stderr, "Fast points test\n");
    for (i=0; i<2; i++)
       state = doFastPoints(screen);
    results[iter][1] = ((double)state*1000)/(SDL_GetTicks() -starttime);

    // Rect fill test
    // 
    starttime=SDL_GetTicks(); 
    fprintf(stderr, "Rect fill test\n");
    state = doRects(screen);
    results[iter][2] = ((double)state*1000)/(SDL_GetTicks() -starttime);
    
    // Blitter test
    // 
    starttime=SDL_GetTicks(); 
    fprintf(stderr, "32x32 Blitter test\n");
    state = doBlitter(screen);
    results[iter][3] = ((double)state*1000)/(SDL_GetTicks() -starttime);
  }

  // Output results
  //
  printf("                          320x240  320x240  640x480  640x480\n");
  printf("                          software hardware software hardware");
  printf("\nSlow points (frames/sec): ");
  for (iter=0; iter<4; iter++)
    if (results[iter][0] == -1)
      printf("   N/A   ");
    else
      printf("%8g ", results[iter][0]);
  printf("\nFast points (frames/sec): ");
  for (iter=0; iter<4; iter++)
    if (results[iter][1] == -1)
      printf("   N/A   ");
    else
      printf("%8g ", results[iter][1]);
  printf("\n   Rect fill (rects/sec): ");
  for (iter=0; iter<4; iter++)
    if (results[iter][2] == -1)
      printf("   N/A   ");
    else
      printf("%8g ", results[iter][2]);
  printf("\n 32x32 blits (blits/sec): ");
  for (iter=0; iter<4; iter++)
    if (results[iter][3] == -1)
      printf("   N/A   ");
    else
      printf("%8g ", results[iter][3]);
  printf("\n");

  
  return 0;
}

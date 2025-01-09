#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"

#define R 0x00
#define G 0x00
#define B 0xff

void fade(SDL_Surface *screen, Uint32 rgb, Uint8 a)
{
	static SDL_Surface *tmp=0;
	if(!screen)
	{
		if(tmp)
			SDL_FreeSurface(tmp);
		tmp=0;
		return;
	}
	if(!tmp)
	{
		tmp=SDL_DisplayFormat(screen);
		SDL_FillRect(tmp,0,rgb);
	}
	SDL_SetAlpha(tmp,SDL_SRCALPHA,a);
	SDL_BlitSurface(tmp,0,screen,0);
}

int main(int argc, char **argv)
{
	SDL_Surface *screen, *image;
	SDL_Event event;
	int done=0;
	int a=128;
	
	/* check arguments */
	if(argc<2 || argc>3)
	{
		printf("%s filename [fade amount 0-255]\n",*argv);
		return 1;
	}

	/* startup SDL */
	if(SDL_Init(SDL_INIT_VIDEO)==-1)
	{
		printf("SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	if(argc>2)
		a=(Uint8)strtol(argv[2],0,10);
	
	/* load the image */
	image=IMG_Load(argv[1]);
	if(!image)
	{
		/* image failed to load */
		printf("IMG_Load: %s\n", IMG_GetError());
		SDL_Quit();
		return 1;
	}

	/* print some info about the image */
	printf("loaded %s: %dx%d %dbpp\n", argv[1],
			image->w, image->h, image->format->BitsPerPixel);

	/* open the screen for displaying the image */
	/* try to match the image size and depth    */
	screen=SDL_SetVideoMode(image->w, image->h, image->format->BitsPerPixel, SDL_ANYFORMAT);
	/* it seems 8bit fades this way fail... */
	/*screen=SDL_SetVideoMode(image->w, image->h, 8, 0);*/
	if(!screen)
	{
		printf("SDL_SetVideoMode: %s\n", SDL_GetError());
		SDL_FreeSurface(image);
		SDL_Quit();
		return 1;
	}

	/* set the window title to the filename */
	SDL_WM_SetCaption(argv[1],argv[1]);

	/* print some info about the obtained screen */
	printf("screen is %dx%d %dbpp\n", screen->w, screen->h, screen->format->BitsPerPixel);
	printf("press cursor up/down to alter the fade amount.\n");
	fprintf(stderr,"fade = %d    \r",a);

	/* do the initial image display */
	SDL_BlitSurface(image,0,screen,0);
	fade(screen,SDL_MapRGB(screen->format,R,G,B),a);
	SDL_Flip(screen);

	/* key repeat so we don't have to keep pressing over and over... */
	SDL_EnableKeyRepeat(1,1);
	
	/* the event loop, redraws if needed, quits on keypress or quit event */
	while(!done && SDL_WaitEvent(&event)!=-1)
	{
		int refresh=0;
		switch(event.type)
		{
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
					case SDLK_UP:
						a--;
						if(a<0) a=0;
						refresh=1;
						break;
					case SDLK_DOWN:
						a++;
						if(a>255) a=255;
						refresh=1;
						break;
					default:
						break;
				}
				if(refresh)
				{
					fprintf(stderr,"fade = %d    \r",a);
					break;
				}
			case SDL_QUIT:
				/* quit events, exit the event loop */
				done=1;
				break;
			case SDL_VIDEOEXPOSE:
				refresh=1;
				break;
			default:
				break;
		}
		if(refresh)
		{
			/* need a redraw, we just redraw the whole screen for simplicity */
			SDL_BlitSurface(image,0,screen,0);
			fade(screen,SDL_MapRGB(screen->format,R,G,B),a);
			SDL_Flip(screen);
		}
	}

	/* free the loaded image */
	SDL_FreeSurface(image);

	/* shutdown SDL */
	SDL_Quit();

	return 0;
}

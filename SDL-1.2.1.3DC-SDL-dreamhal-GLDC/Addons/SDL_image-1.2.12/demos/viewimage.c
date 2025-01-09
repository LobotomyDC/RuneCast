#include "SDL.h"
#include "SDL_image.h"

int main(int argc, char **argv)
{
	SDL_Surface *screen, *image;
	SDL_Event event;
	int done=0;
	
	/* check arguments */
	if(argc!=2)
	{
		printf("%s filename\n",*argv);
		return 1;
	}

	/* startup SDL */
	if(SDL_Init(SDL_INIT_VIDEO)==-1)
	{
		printf("SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

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

	/* do the initial image display */
	SDL_BlitSurface(image,0,screen,0);
	SDL_Flip(screen);

	/* the event loop, redraws if needed, quits on keypress or quit event */
	while(!done && SDL_WaitEvent(&event)!=-1)
	{
		switch(event.type)
		{
			case SDL_KEYDOWN:
			case SDL_QUIT:
				/* quit events, exit the event loop */
				done=1;
				break;
			case SDL_VIDEOEXPOSE:
				/* need a redraw, we just redraw the whole screen for simplicity */
				SDL_BlitSurface(image,0,screen,0);
				SDL_Flip(screen);
				break;
			default:
				break;
		}
	}

	/* free the loaded image */
	SDL_FreeSurface(image);

	/* shutdown SDL */
	SDL_Quit();

	return 0;
}

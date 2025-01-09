#include<kos.h>
#include<stdio.h>
#include "SDLDH/SDL.h"
#include "SDLDH/SDL_dreamcast.h"

#define NFRAMES 1000

/* Real coords (texture width and height) for hardware scaled. */
#define TXR_X 512
#define TXR_Y 256

/* Screen coords */
#define SCR_X 320
#define SCR_Y 240


static SDL_Surface *screen;

void testme(char *flags)
{
	int tick=0;
	Uint32 inicio,final;

	SDL_FillRect(screen,NULL,-1);
	fflush(stdout); SDL_Delay(100);
	inicio=SDL_GetTicks();
	while(inicio==SDL_GetTicks());
	while(tick<NFRAMES)
	{

		SDL_Rect rect;
		rect.x=2;
		rect.y=2;
		rect.w=SCR_X-4;
		rect.h=SCR_Y-4;
		SDL_FillRect(screen,&rect,tick);

/* CAUTION DIRECT ACCESS: line pitch no always is width*bytes_per_color
		unsigned short *buf=screen->pixels;
		int i,j;
		for(i=0;i<SCR_Y;i++)
			for(j=0;j<SCR_X;j++)
				buf[j+i*(screen->pitch>>1)]=tick+i;
*/
		SDL_Flip(screen);
		tick++;
	}
	final=SDL_GetTicks()-1;
	printf(" - '%s' -> Ticks=%i, FPS=%i\n",flags,final-inicio, (NFRAMES*1000)/(final-inicio));
}

/* HARDWARE SCALED: ONLY FOR TEXTURED DRIVER !
   NO FULLSCREEN
   IF FULLSCREEN + TEXTURED_DRIVER = DMA_DRIVER */
void hardware_scaled_test()
{
	screen=SDL_SetVideoMode(TXR_X,TXR_Y,16,SDL_HWSURFACE);
	SDL_DC_SetWindow(SCR_X,SCR_Y);
	testme("SDL_HWSURFACE");
}


/* HARDWARE SCALED AND DOUBLEBUF: ONLY FOR TEXTURED DRIVER ! */
void hardware_scaled_double_buffer_test()
{
	screen=SDL_SetVideoMode(TXR_X,TXR_Y,16,SDL_HWSURFACE|SDL_DOUBLEBUF);
	SDL_DC_SetWindow(SCR_X,SCR_Y);
	testme("SDL_HWSURFACE|SDL_DOUBLEBUF");
}

/* DOUBLE BUFFER : FAST !!!! */
void double_buffer_test()
{
	screen=SDL_SetVideoMode(SCR_X,SCR_Y,16,SDL_FULLSCREEN|SDL_DOUBLEBUF|SDL_HWSURFACE);
	testme("SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN");
}

/* NORMAL */
void normal_test()
{
	screen=SDL_SetVideoMode(SCR_X,SCR_Y,16,SDL_FULLSCREEN|SDL_HWSURFACE);
	testme("SDL_HWSURFACE|SDL_FULLSCREEN");
}

int main()
{
	    cdrom_init();
    /* Do an initial initialization */
    cdrom_reinit();
    puts("\n\n * SDL-DC : Video drivers test.");

    SDL_DC_ShowAskHz(SDL_FALSE);
    SDL_DC_Default60Hz(SDL_FALSE);
    SDL_DC_VerticalWait(SDL_FALSE);
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) >= 0)
    {
	puts("\n-------------------\n| TEXTURED DRIVER |\n-------------------");
	SDL_DC_SetVideoDriver(SDL_DC_TEXTURED_VIDEO);
	hardware_scaled_test();
	hardware_scaled_double_buffer_test();
	puts("\n-----------------\n| DIRECT DRIVER |\n-----------------");
	SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
	double_buffer_test();
	normal_test();
	puts("\n--------------\n| DMA DRIVER |\n--------------");
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
	double_buffer_test();
	normal_test();
        SDL_Quit();
    }
    return 0;
} 

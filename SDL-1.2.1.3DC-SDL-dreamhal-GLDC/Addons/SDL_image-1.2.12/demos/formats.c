#include "SDL.h"
#include "SDL_image.h"

char *files[]={
	"lena.bmp", "BMP",
	"lena.cur", "CUR",
	"lena.gif", "GIF",
	"lena.ico", "ICO",
	"lena.jpg", "JPG",
	"lena.lbm", "LBM",
	"lena.pcx", "PCX",
	"lena.png", "PNG",
	"lena.pnm", "PNM",
	"lena.tga", "TGA",
	"lena.tif", "TIF",
	"lena.xcf", "XCF",
	"lena.xpm", "XPM",
	"lena.xv",  "XV",
	NULL,
	"lena.xbm", "XBM",
	NULL
};

void print_init_flags(int flags)
{
	if(flags&IMG_INIT_JPG)
		printf("JPG ");
	if(flags&IMG_INIT_PNG)
		printf("PNG ");
	if(flags&IMG_INIT_TIF)
		printf("TIF ");
	if(!flags)
		printf("None");
	printf("\n");
}

int main(int argc, char **argv)
{
	SDL_Surface *image;
	int i;
	
	/* check arguments */
	if(argc!=1)
	{
		printf("%s (no arguments)\n",*argv);
		return 1;
	}


	/* do a version check for information */
	{
		SDL_version compile_version;
		const SDL_version *link_version=IMG_Linked_Version();
		SDL_IMAGE_VERSION(&compile_version);
		printf("compiled with SDL_image version: %d.%d.%d\n", 
				compile_version.major,
				compile_version.minor,
				compile_version.patch);
		printf("running with SDL_image version: %d.%d.%d\n", 
				link_version->major,
				link_version->minor,
				link_version->patch);
	}

	/* startup SDL */
	if(SDL_Init(SDL_INIT_VIDEO)==-1)
	{
		printf("SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	int initted=IMG_Init(0);
	printf("Before IMG_Init SDL_image supported: ");
	print_init_flags(initted);
	initted=IMG_Init(~0); /*~0 is all bits set */
	printf("After  IMG_Init SDL_image supported: ");
	print_init_flags(initted);

	for(i=0; files[i]; i+=2)
	{
		/* load the image */
		image=IMG_Load(files[i]);
		if(!image)
		{
			/* image failed to load */
			printf("IMG_Load: %s\n", IMG_GetError());
			printf("%s is not supported, or the image data is corrupted\n", files[i+1]);
			files[i+1]=NULL;
			continue;
		}
		/* report success */
		printf("%s is supported\n", files[i+1]);
		/* free the loaded image */
		SDL_FreeSurface(image);
	}

	/* shutdown SDL_image */

	/* shutdown SDL */
	SDL_Quit();

	return 0;
}

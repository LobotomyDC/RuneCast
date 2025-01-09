

//Slide show Example BY Ian Micheal 2024

#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "include/SDL.h"    
#include "include/SDL_image.h"
#include "include/SDL_dreamcast.h" 

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

#define NUM_FORMATS 4
#define DISPLAY_TIME 1000 // 5 seconds
#define FRAME_DELAY 100   // 100 ms between GIF frames
#define MAX_DEBUG_MSG_LEN 256
#define MAX_FILEPATH_LEN 64

#define SCR_X 640
#define SCR_Y 480

const char* image_extensions[NUM_FORMATS] = {".bmp", ".jpg", ".png", ".gif"};

void debug_print(const char* message) {
    printf("%s\n", message);
    fflush(stdout);
}

void print_memory_stats(const char* label) {
    printf("\n%s:\n", label);
    malloc_stats();
    fflush(stdout);
}

SDL_Surface* convert_to_display_format(SDL_Surface* screen, SDL_Surface* image) {
    SDL_Surface* converted = SDL_DisplayFormat(image);
    if (converted == NULL) {
        debug_print("Failed to convert surface to display format");
    } else {
        debug_print("Converted to display format successfully");
    }
    return converted;
}

void display_image(SDL_Surface* screen, SDL_Surface* image, int duration) {
    SDL_Surface* display_image;
    SDL_Rect dest;
    Uint32 start_time, current_time;

    debug_print("Entering display_image function");

    display_image = convert_to_display_format(screen, image);
    if (!display_image) {
        debug_print("Failed to convert image to display format");
        return;
    }

    if (SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0)) != 0) {
        debug_print("Failed to clear screen");
    }

    dest.x = (SCR_X - display_image->w) / 2;
    dest.y = (SCR_Y - display_image->h) / 2;
    dest.w = display_image->w;
    dest.h = display_image->h;

    if (SDL_BlitSurface(display_image, NULL, screen, &dest) < 0) {
        debug_print("BlitSurface failed");
    } else {
        debug_print("BlitSurface succeeded");
    }

    if (SDL_Flip(screen) < 0) {
        debug_print("Flip failed");
    } else {
        debug_print("Flip succeeded");
    }

    debug_print("Starting display loop");
    start_time = SDL_GetTicks();
    do {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ||
                (event.type == SDL_JOYBUTTONDOWN && event.jbutton.button == 3)) {
                debug_print("Exit event received");
                SDL_FreeSurface(display_image);
                return;
            }
        }
        current_time = SDL_GetTicks();
        SDL_Delay(10);
    } while (current_time - start_time < duration);

    debug_print("Display loop completed");
    SDL_FreeSurface(display_image);
    debug_print("Exiting display_image function");
}

void display_gif(SDL_Surface* screen, const char* filepath) {
    SDL_RWops* rwops = SDL_RWFromFile(filepath, "rb");
    if (!rwops) {
        debug_print("Failed to open GIF file");
        return;
    }

    SDL_Surface* gif_surface = NULL;
    Uint32 start_time = SDL_GetTicks();
    Uint32 current_time;
    int frame_count = 0;
    int loop_count = 0;
    char debug_msg[MAX_DEBUG_MSG_LEN];

    debug_print("Starting GIF display");

    while (current_time - start_time < DISPLAY_TIME) {
        Sint64 current_pos = SDL_RWtell(rwops);
        snprintf(debug_msg, sizeof(debug_msg), "Current file position: %lld", current_pos);
        debug_print(debug_msg);

        gif_surface = IMG_LoadGIF_RW(rwops);
        if (!gif_surface) {
            snprintf(debug_msg, sizeof(debug_msg), "Failed to load GIF frame. Error: %s", IMG_GetError());
            debug_print(debug_msg);
            
            SDL_RWseek(rwops, 0, RW_SEEK_SET);
            loop_count++;
            snprintf(debug_msg, sizeof(debug_msg), "GIF reset. Loop count: %d", loop_count);
            debug_print(debug_msg);
            continue;
        }

        frame_count++;
        snprintf(debug_msg, sizeof(debug_msg), "Loaded frame %d. Size: %dx%d, BPP: %d", 
                 frame_count, gif_surface->w, gif_surface->h, gif_surface->format->BitsPerPixel);
        debug_print(debug_msg);

        display_image(screen, gif_surface, FRAME_DELAY);
        SDL_FreeSurface(gif_surface);

        current_time = SDL_GetTicks();

        // Check for exit conditions
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ||
                (event.type == SDL_JOYBUTTONDOWN && event.jbutton.button == 3)) {
                debug_print("Exit event received during GIF display");
                SDL_RWclose(rwops);
                return;
            }
        }

        snprintf(debug_msg, sizeof(debug_msg), "Frame %d displayed. Time elapsed: %u ms", 
                 frame_count, current_time - start_time);
        debug_print(debug_msg);

        SDL_Delay(FRAME_DELAY);
    }

    SDL_RWclose(rwops);
    debug_print("GIF display completed");
    snprintf(debug_msg, sizeof(debug_msg), "Total frames displayed: %d, Loops: %d", frame_count, loop_count);
    debug_print(debug_msg);
}

int main(int argc, char *argv[]) {
    SDL_Surface *screen, *image;
    char filepath[MAX_FILEPATH_LEN];
    char debug_msg[MAX_DEBUG_MSG_LEN];
    int i;

    debug_print("Program started");
    print_memory_stats("Initial memory state");

    SDL_DC_ShowAskHz(SDL_FALSE);
    SDL_DC_Default60Hz(SDL_FALSE);
    SDL_DC_VerticalWait(SDL_FALSE);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        snprintf(debug_msg, sizeof(debug_msg), "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        debug_print(debug_msg);
        return 1;
    }

    puts("\n--------------\n| DMA DRIVER |\n--------------");
    SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);

    screen = SDL_SetVideoMode(SCR_X, SCR_Y, 16, SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE);
    if (screen == NULL) {
        snprintf(debug_msg, sizeof(debug_msg), "Could not create window: %s", SDL_GetError());
        debug_print(debug_msg);
        return 1;
    }

    print_memory_stats("After SDL initialization");

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Flip(screen);
    SDL_Delay(1000);

    for (i = 0; i < NUM_FORMATS; i++) {
        snprintf(filepath, sizeof(filepath), "/rd/image%s", image_extensions[i]);
        snprintf(debug_msg, sizeof(debug_msg), "Attempting to load: %.200s", filepath);
        debug_print(debug_msg);
        
        print_memory_stats("Before loading image");

        if (strcmp(image_extensions[i], ".gif") == 0) {
            display_gif(screen, filepath);
        } else {
            image = IMG_Load(filepath);
            if (image == NULL) {
                snprintf(debug_msg, sizeof(debug_msg), "Unable to load image %.100s! SDL_image Error: %.100s", 
                         filepath, IMG_GetError());
                debug_print(debug_msg);
                continue;
            }

            snprintf(debug_msg, sizeof(debug_msg), "Image loaded successfully. Size: %dx%d, BPP: %d", 
                     image->w, image->h, image->format->BitsPerPixel);
            debug_print(debug_msg);

            display_image(screen, image, DISPLAY_TIME);
            SDL_FreeSurface(image);
            debug_print("Image freed");
        }

        print_memory_stats("After freeing image");
    }

    SDL_Quit();
    print_memory_stats("Final memory state");
    debug_print("Program ended");
    return 0;
}
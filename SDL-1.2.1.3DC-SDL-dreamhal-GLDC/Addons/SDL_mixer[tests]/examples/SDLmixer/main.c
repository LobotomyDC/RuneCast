/********************************************************************************************/
/* KallistiOS SDL-DREAMHAL Version 1.0 (**SDL Version:** 1.2.1.3DC (Framebuffer & OpenGL)   */
/********************************************************************************************/
/********************************************************************************************/
/* Name:     main.c                                                                         */
/* Title:    KallistiOS **SDL Version:** 1.2.1.3DC (Framebuffer) SDLMIXER TEST              */
/* Author:   (c)Ian Micheal                                                                 */
/* Created:  10/5/24                                                                        */
/*                                                                                          */
/* Version:  1.0                                                                            */
/* Platform: Dreamcast | KallistiOS:2.0 | SDL DMA |FRAMEBUFFER|GCC13.2                      */
/*                                                                                          */
/* Description:                                                                             */
/* Tests font display music midi,mod,wav analog control and dpad and dc controller          */
/* shows animation control sound music and midi music                                       */
/* History: version 1 - Added SDLMIXER TEST                                                 */
/********************************************************************************************/



#include "include/SDL.h"
#include "include/SDL_image.h"
#include "include/SDL_mixer.h"
#include "include/SDL_dreamcast.h" 
#include <kos.h>
#include <math.h>
#include <stdio.h>

#include "SFont.h"
#include "font.h"

KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);
#define ANALOG_MAX 32767.0f
#define DEAD_ZONE 20
#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#define FPS 60
#define NUM_FRAMES 6
#define MOVE_SPEED 10.0f

int NextTick, interval;
Mix_Music *mus, *mus2;
Mix_Chunk *wav, *wav2;
SDL_Rect animRect = { SCR_WIDTH / 2, SCR_HEIGHT / 2, 0, 0 };
int dpad_x = 0;
int dpad_y = 0;
SDL_Surface *font_surface;
SFont_FontInfo font;
char button_name[32] = "";
Uint32 button_press_time = 0;

void FPS_Initial(void) {
    NextTick = 0;
    interval = 1000 / FPS;
}

void FPS_Fn(void) {
    if (NextTick > SDL_GetTicks()) SDL_Delay(NextTick - SDL_GetTicks());
    NextTick = SDL_GetTicks() + interval;
}

SDL_Surface *ImgLoader(const char *file, SDL_bool bCKey, int r, int g, int b, int alpha) {
    SDL_Surface *pic = IMG_Load(file);
    if (pic == NULL) {
        printf("Missing image %s : %s\n", file, IMG_GetError());
        return NULL;
    }
    if (bCKey) {
        SDL_SetColorKey(pic, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(pic->format, r, g, b));
    }
    if (alpha) SDL_SetAlpha(pic, SDL_SRCALPHA | SDL_RLEACCEL, 255 - alpha);
    SDL_Surface *converted = SDL_DisplayFormat(pic);
    SDL_FreeSurface(pic);
    return converted;
}

void InitFont() {
    SDL_Surface *tmp;
    tmp = IMG_Load_RW(SDL_RWFromMem((void*)&font_png, FONT_PNG_SIZE), 0);
    font_surface = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);
    font.Surface = font_surface;
    SFont_InitFontInfo(&font);
}

void clearevents(void) {
    SDL_Event event;
    SDL_Delay(100);
    while(SDL_PollEvent(&event))
        SDL_Delay(20);
}

int HandleEvent() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYBUTTONDOWN:
                switch (event.jbutton.button) {
                    case 2:  // A button
                        strcpy(button_name, "A Button");
                        Mix_PlayMusic(mus, -1);
                        break;
                    case 1:  // B button
                        strcpy(button_name, "B Button");
                        Mix_PlayMusic(mus2, -1);
                        break;
                    case 6:  // X button
                        strcpy(button_name, "X Button");
                        Mix_PlayChannel(-1, wav, 0);
                        break;
                    case 5:  // Y button
                        strcpy(button_name, "Y Button");
                        Mix_PlayChannel(-1, wav2, 0);
                        break;
                    case 13:  // Start button
                        strcpy(button_name, "Start Button - Exiting");
                        return 1;  // Exit the program
                    default:
                        sprintf(button_name, "Button %d", event.jbutton.button);
                        break;
                }
                button_press_time = SDL_GetTicks();
                break;
            case SDL_JOYHATMOTION:
                dpad_x = 0;
                dpad_y = 0;
                if (event.jhat.value & SDL_HAT_UP) dpad_y = -1;
                if (event.jhat.value & SDL_HAT_DOWN) dpad_y = 1;
                if (event.jhat.value & SDL_HAT_LEFT) dpad_x = -1;
                if (event.jhat.value & SDL_HAT_RIGHT) dpad_x = 1;
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

int main(int argc, char *argv[]) {
    SDL_Surface *screen;
    SDL_Surface *anim[NUM_FRAMES];
    int pidx = 0, aTick = 0;
    SDL_Joystick *joystick;
    char msg[1024];

    printf("Initializing...\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_DC_ShowAskHz(SDL_FALSE);
    SDL_DC_Default60Hz(SDL_FALSE);
    SDL_DC_VerticalWait(SDL_TRUE);

    SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);

    puts("\n--------------\n| DMA DRIVER |\n--------------");
    SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);

    screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 16, SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE);
    if (screen == NULL) {
        fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);
    if (!joystick) {
        fprintf(stderr, "Failed to open joystick!\n");
    } else {
        printf("Joystick opened successfully\n");
    }

    InitFont();

    printf("Loading animation frames...\n");
    const char* anim_files[] = {
        "/rd/anim/justice1.gif", "/rd/anim/justice2.gif", "/rd/anim/justice3.gif",
        "/rd/anim/justice4.gif", "/rd/anim/justice5.gif", "/rd/anim/justice6.gif"
    };
    for (int i = 0; i < NUM_FRAMES; i++) {
        anim[i] = ImgLoader(anim_files[i], SDL_TRUE, 255, 255, 255, 0);
        if (anim[i] == NULL) {
            fprintf(stderr, "Failed to load animation frame %d\n", i);
            SDL_Quit();
            return 1;
        }
    }

    printf("Initializing audio...\n");
    if (Mix_OpenAudio(44100, AUDIO_S16, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer initialization failed: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    mus = Mix_LoadMUS("/rd/mixer/ff2prlde.mid");
    mus2 = Mix_LoadMUS("/rd/mixer/jungle.mod");
    wav = Mix_LoadWAV("/rd/mixer/po_p2k.wav");
    wav2 = Mix_LoadWAV("/rd/mixer/start.wav");

    if (!mus || !mus2 || !wav || !wav2) {
        fprintf(stderr, "Failed to load audio files\n");
        SDL_Quit();
        return 1;
    }

    Mix_PlayMusic(mus, -1);

    FPS_Initial();
    clearevents();

    printf("Entering main loop...\n");
    int quit = 0;
    while (!quit) {
        aTick++;
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 16, 16, 32));
        SDL_BlitSurface(anim[pidx], NULL, screen, &animRect);

        // Display button name at the top of the screen
        if (SDL_GetTicks() - button_press_time < 2000 && strlen(button_name) > 0) {
            SFont_XCenteredStringInfo(screen, &font, font_surface->h, button_name);
        }

        // Display animation position at the bottom of the screen
        sprintf(msg, "(%i,%i)", animRect.x, animRect.y);
        SFont_XCenteredStringInfo(screen, &font, SCR_HEIGHT - font_surface->h, msg);

        SDL_Flip(screen);
        
        if (aTick % 10 == 0) pidx = (pidx + 1) % NUM_FRAMES;

        if (HandleEvent()) {
            quit = 1;
        }

       
        SDL_JoystickUpdate();

        // Handle analog stick input
        Sint16 analogX = SDL_JoystickGetAxis(joystick, 0);
        Sint16 analogY = SDL_JoystickGetAxis(joystick, 1);

        float normalizedX = 0.0f;
        float normalizedY = 0.0f;

        // Apply dead zone
       if (abs(analogX) > DEAD_ZONE) {
       normalizedX = (float)(analogX) / ANALOG_MAX; // Normalize directly by full range
}
        if (abs(analogY) > DEAD_ZONE) {
            normalizedY = (float)(analogY - (analogY > 0 ? DEAD_ZONE : -DEAD_ZONE)) / (ANALOG_MAX - DEAD_ZONE);
        }

        // Apply analog movement directly, use float for precision
       float newX = animRect.x + (normalizedX * MOVE_SPEED * 100);  // Multiply by 100 for more noticeable movement
       float newY = animRect.y + (normalizedY * MOVE_SPEED * 100);

        // Apply D-pad movement
        newX += dpad_x * MOVE_SPEED;
        newY += dpad_y * MOVE_SPEED;

        // Clamp player position to screen boundaries
        animRect.x = (int)fmaxf(0, fminf(SCR_WIDTH - animRect.w, newX));
        animRect.y = (int)fmaxf(0, fminf(SCR_HEIGHT - animRect.h, newY));

        // Debug output (updated in place)
        printf("\rAnalog X: %5d, Y: %5d | Norm X: %.4f, Y: %.4f | Player X: %3d, Y: %3d", 
               analogX, analogY, normalizedX, normalizedY, animRect.x, animRect.y);
        fflush(stdout);

        FPS_Fn();
    }

    printf("\nCleaning up...\n");
    for (int i = 0; i < NUM_FRAMES; i++) {
        SDL_FreeSurface(anim[i]);
    }
    Mix_FreeChunk(wav);
    Mix_FreeChunk(wav2);
    Mix_FreeMusic(mus);
    Mix_FreeMusic(mus2);
    Mix_CloseAudio();
    if (joystick) {
        SDL_JoystickClose(joystick);
    }
    SDL_FreeSurface(font_surface);
    SDL_Quit();

    printf("Program ended.\n");
    return 0;
}
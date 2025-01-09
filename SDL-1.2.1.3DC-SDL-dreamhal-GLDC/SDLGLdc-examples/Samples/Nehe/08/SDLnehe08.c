/*  DREAMCAST
 *IAN MICHEAL Ported SDL+OPENGL USING SDL[DREAMHAL][GLDC][KOS2.0]2021
 *IAN MICHEAL Update up port and code fixes for GCC13.2 and NEW KOS2.10
 * Cleaned and tested on dreamcast hardware by Ianmicheal
 *		This Code Was Created By Pet & Commented/Cleaned Up By Jeff Molofee
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit NeHe Productions At http://nehe.gamedev.net
 */

#include <math.h>			// Header File For Windows Math Library
#include <stdio.h>			// Header File For Standard Input/Output
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glkos.h"
#include "SDLDH/SDL.h"
#include "SDLDH/SDL_dreamcast.h"
#include <kos.h>
#define FPS 60
#define ROTATION_SPEED 1.0f
#define ZOOM_SPEED 0.05f

static Uint32 waittime = 1000/FPS;
static Uint32 framestarttime = 0;
static Sint32 delaytime;
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

static int light = 0;
static GLfloat xrot, yrot;
static GLfloat xspeed = 0.0f, yspeed = 0.0f;
static GLfloat z = -5.0f;

static const GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
static const GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

static GLuint filter = 0;
static GLuint texture[3];
static GLuint blend = 0;

static SDL_Surface *LoadBMP(const char *filename)
{
    Uint8 *rowhi, *rowlo;
    Uint8 *tmpbuf, tmpch;
    SDL_Surface *image = SDL_LoadBMP(filename);
    if (!image) {
        fprintf(stderr, "Unable to load %s: %s\n", filename, SDL_GetError());
        return NULL;
    }

    tmpbuf = (Uint8 *)malloc(image->pitch);
    if (!tmpbuf) {
        fprintf(stderr, "Out of memory\n");
        SDL_FreeSurface(image);
        return NULL;
    }

    rowhi = (Uint8 *)image->pixels;
    rowlo = rowhi + (image->h * image->pitch) - image->pitch;
    for (int i = 0; i < image->h / 2; ++i) {
        for (int j = 0; j < image->w; ++j) {
            tmpch = rowhi[j*3];
            rowhi[j*3] = rowhi[j*3+2];
            rowhi[j*3+2] = tmpch;
            tmpch = rowlo[j*3];
            rowlo[j*3] = rowlo[j*3+2];
            rowlo[j*3+2] = tmpch;
        }
        memcpy(tmpbuf, rowhi, image->pitch);
        memcpy(rowhi, rowlo, image->pitch);
        memcpy(rowlo, tmpbuf, image->pitch);
        rowhi += image->pitch;
        rowlo -= image->pitch;
    }
    free(tmpbuf);
    return image;
}

static void LoadGLTextures(void)
{
    SDL_Surface *image1 = LoadBMP("/rd/glass.bmp");
    if (!image1) {
        SDL_Quit();
        exit(1);
    }

    glGenTextures(3, texture);

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->w, image1->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->w, image1->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image1->w, image1->h, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);

    SDL_FreeSurface(image1);
}

static void InitGL(int Width, int Height)
{
    printf("Entering InitGL\n");
    LoadGLTextures();
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    printf("Exiting InitGL\n");
}

static void DrawGLScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, z);

    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);

    glBindTexture(GL_TEXTURE_2D, texture[filter]);

    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    // Top Face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    // Bottom Face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) 
{
    printf("Entering main\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL initialized\n");

    SDL_DC_ShowAskHz(SDL_FALSE);
    SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);
    SDL_DC_MapKey(0, SDL_DC_A, SDLK_l);
    SDL_DC_MapKey(0, SDL_DC_Y, SDLK_b);
    SDL_DC_MapKey(0, SDL_DC_X, SDLK_f);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    printf("Attempting to set video mode\n");
    if (!SDL_SetVideoMode(640, 480, 0, SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF)) {
        fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Video mode set successfully\n");

    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption("OpenGL Cube Demo", NULL);

    printf("Calling InitGL\n");
    InitGL(640, 480);
    printf("InitGL completed\n");

    Uint32 frameCount = 0;
    Uint32 fpsTimer = SDL_GetTicks();
    int done = 0;

    printf("Entering main loop\n");
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            } else if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE: done = 1; break;
                    case SDLK_b:
                        blend = !blend;
                        if (blend) {
                            glEnable(GL_BLEND);
                            glDisable(GL_DEPTH_TEST);
                        } else {
                            glDisable(GL_BLEND);
                            glEnable(GL_DEPTH_TEST);
                        }
                        break;
                    case SDLK_l:
                        light = !light;
                        if (light) glEnable(GL_LIGHTING);
                        else glDisable(GL_LIGHTING);
                        break;
                    case SDLK_f:
                        filter = (filter + 1) % 3;
                        break;
                }
            }
        }

        const Uint8 *keys = SDL_GetKeyState(NULL);
        if (keys[SDLK_PAGEUP])   z -= ZOOM_SPEED;
        if (keys[SDLK_PAGEDOWN]) z += ZOOM_SPEED;
        if (keys[SDLK_UP])       xspeed -= ROTATION_SPEED;
        if (keys[SDLK_DOWN])     xspeed += ROTATION_SPEED;
        if (keys[SDLK_LEFT])     yspeed -= ROTATION_SPEED;
        if (keys[SDLK_RIGHT])    yspeed += ROTATION_SPEED;

        xrot += xspeed;
        yrot += yspeed;

        DrawGLScene();

        Uint32 current_time = SDL_GetTicks();
        Sint32 time_left = waittime - (current_time - framestarttime);
        if (time_left > 0) {
            SDL_Delay(time_left);
        }
        framestarttime = SDL_GetTicks();

        frameCount++;
        if (SDL_GetTicks() - fpsTimer > 1000) {
            printf("FPS: %d, xspeed: %f, yspeed: %f\n", frameCount, xspeed, yspeed);
            frameCount = 0;
            fpsTimer = SDL_GetTicks();
        }
    }

    printf("Exiting main loop\n");
    SDL_Quit();
    printf("SDL quit\n");
    return 0;
}

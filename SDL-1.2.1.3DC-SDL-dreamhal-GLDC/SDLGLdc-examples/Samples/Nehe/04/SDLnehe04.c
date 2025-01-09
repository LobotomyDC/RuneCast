/*  DREAMCAST
 *IAN MICHEAL Ported SDL+OPENGL USING SDL[DREAMHAL][GLDC][KOS2.0]2021
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
Uint32 waittime = 1000.0f/FPS;
Uint32 framestarttime = 0;
Sint32 delaytime;
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

/* rotation angle for the triangle and quadrilateral. */
float rtri = 0.0f;
float rquad = 0.0f;

/* rotation speed */
float tri_speed = 100.2f;
float quad_speed = 100.5f;

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */
void DrawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(-1.5f,0.0f,-6.0f);
    glRotatef(rtri,0.0f,1.0f,0.0f);

    // draw a triangle (in smooth coloring mode)
    glBegin(GL_POLYGON);
        glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);
        glColor3f(0.0f,1.0f,0.0f); glVertex3f( 1.0f,-1.0f, 0.0f);
        glColor3f(0.0f,0.0f,1.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
    glEnd();

    glLoadIdentity();
    glTranslatef(1.5f,0.0f,-6.0f);
    glRotatef(rquad,1.0f,0.0f,0.0f);

    // draw a square (quadrilateral)
    glColor3f(0.5f,0.5f,1.0f);
    glBegin(GL_QUADS);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
    glEnd();

    rtri  += tri_speed;
    rquad -= quad_speed;

    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) 
{  
    int done;
    Uint8 *keys;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_DC_ShowAskHz(SDL_FALSE);
    SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);

    if (SDL_SetVideoMode(640, 480, 0, SDL_OPENGL) == NULL) {
        fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);
    SDL_ShowCursor(0);

    SDL_WM_SetCaption("Jeff Molofee's GL Code Tutorial ... NeHe '99", NULL);

    InitGL(640, 480);
    done = 0;

    while (!done) {
        DrawGLScene();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                done = 1;
            }
        }

        keys = SDL_GetKeyState(NULL);
        
        if (keys[SDLK_UP])    tri_speed += 0.01f;
        if (keys[SDLK_DOWN])  tri_speed -= 0.01f;
        if (keys[SDLK_RIGHT]) quad_speed += 0.01f;
        if (keys[SDLK_LEFT])  quad_speed -= 0.01f;

        Uint32 current_time = SDL_GetTicks();
        Sint32 time_left = waittime - (current_time - framestarttime);
        if (time_left > 0) {
            SDL_Delay((Uint32)time_left);
        }
        framestarttime = SDL_GetTicks();
    }

    SDL_Quit();
    return 0;
}
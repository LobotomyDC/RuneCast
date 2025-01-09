
# 🎮 DREAMCAST SDL-DREAMHAL Version 1.0 SDL_gl.a

![Version](https://img.shields.io/badge/Version-1.0-blue)
![Platform](https://img.shields.io/badge/Platform-Dreamcast-orange)
![License](https://img.shields.io/badge/License-MIT-green)
![OpenGL](https://img.shields.io/badge/OpenGL-Supported-brightgreen)

---

### 🚀 Main Program Example

Here’s an example of setting up **SDL with OpenGL** on the Dreamcast, mapping PC keyboard keys to the Dreamcast controller, and enabling analog stick emulation for the mouse. This example also highlights some custom video and input functions specific to Dreamcast SDL.

```c
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glkos.h"
#include "SDLDH/SDL.h"
#include "SDLDH/SDL_dreamcast.h"

int main(int argc, char **argv) 
{  
    int done;
    Uint8 *keys;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_DC_ShowAskHz(SDL_FALSE);

    // Dreamcast-specific key mappings
    SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);
    SDL_DC_MapKey(0, SDL_DC_A, SDLK_t);
    SDL_DC_MapKey(0, SDL_DC_L, SDLK_PAGEUP);   // Map Left Trigger to Page Up
    SDL_DC_MapKey(0, SDL_DC_R, SDLK_PAGEDOWN); // Map Right Trigger to Page Down

    if (SDL_SetVideoMode(640, 480, 0, SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF) == NULL) {
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
            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        done = 1;
                        break;
                    case SDLK_t:
                        twinkle = !twinkle;
                        break;
                }
            }
        }

        keys = SDL_GetKeyState(NULL);
        if (keys[SDLK_PAGEUP])   zoom -= 1.02f; // Left Trigger (L)
        if (keys[SDLK_PAGEDOWN]) zoom += 1.02f; // Right Trigger (R)
        if (keys[SDLK_UP])       tilt -= 1.5f;
        if (keys[SDLK_DOWN])     tilt += 1.5f;

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
```

---

### 🎨 OpenGL Setup for Dreamcast

- **Correct OpenGL headers:**

  Make sure you use the Dreamcast-specific OpenGL headers included with SDL-DREAMHAL:

  ```c
  #include "GL/gl.h"
  #include "GL/glu.h"
  #include "GL/glkos.h"
  ```

- **Video mode setup:**

  The following settings are used to initialize an OpenGL context:

  ```c
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  ```

  This ensures a double-buffered OpenGL context with a 16-bit depth buffer.

---

### 🕹️ Dreamcast Controller Mappings

You can map any PC keyboard key to the Dreamcast controller using `SDL_DC_MapKey`. The following are examples of button mappings:

```c
SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);   // Start -> ESC
SDL_DC_MapKey(0, SDL_DC_A, SDLK_t);            // A -> T
SDL_DC_MapKey(0, SDL_DC_L, SDLK_PAGEUP);       // Left Trigger -> Page Up
SDL_DC_MapKey(0, SDL_DC_R, SDLK_PAGEDOWN);     // Right Trigger -> Page Down
```

### Event Handling and Input

The Dreamcast joystick (controller) can emulate keyboard and mouse inputs. For example:

```c
SDL_DC_EmulateKeyboard(SDL_TRUE);  // Enable keyboard emulation
SDL_DC_EmulateMouse(SDL_TRUE);     // Enable mouse emulation
```

---

### 🖼️ Other Video Functions

Here are some useful video-related functions specific to the Dreamcast:

```c
SDL_DC_SetWindow(int width, int height);   // For textured video only
SDL_DC_VerticalWait(SDL_bool value);       // Enable/disable vertical retrace wait
SDL_DC_ShowAskHz(SDL_bool value);          // Enable/disable 50/60Hz choice (PAL only)
SDL_DC_Default60Hz(SDL_bool value);        // Set default to 60Hz (PAL only)
```

---

### 🔊 Audio Functions

```c
SDL_DC_SetSoundBuffer(void *buffer);       // Set custom internal sound buffer
SDL_DC_RestoreSoundBuffer(void);           // Reset to default sound buffer
```

**💡 Tip:** Using a custom sound buffer can help improve performance by reducing memory copying in the sound callback.

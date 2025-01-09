# 🎮 DREAMCAST SDL-DREAMHAL Version 1.0 (**SDL Version:** 1.2.1.3DC (Framebuffer & OpenGL)

![KallistiOS Version](https://img.shields.io/badge/KallistiOS-v2.1.0-blue)
![SDL Version](https://img.shields.io/badge/SDL-1.2.1.3DC-green)
![GLdc Version](https://img.shields.io/badge/GLdc-Latest-orange)

---

### 📦 What is this ?
**DREAMCAST SDL-DREAMHAL Version 1.0** is designed to provide a seamless environment for developing Dreamcast homebrew applications using **KallistiOS** and **SDL**.

## 📚 Table of Contents
- [Overview](#-overview)
- [Features](#-features)
- [Performance Comparison](#-performance-comparison)
- [Dreamcast Integration](#-dreamcast-integration)
- [SDL](#-sdl)
- [New Features](#-new-features)

## 🚀 Overview

SDL-DREAMHAL is a customized version of SDL (Simple DirectMedia Layer) tailored for the Dreamcast. It leverages GLDC for rendering, providing significant performance enhancements for 2D blitting and other operations.

- **Based on SDL 1.2.13**
- **Tested and working on KallistiOS v2.1.0**
- **Compiler: sh-elf-gcc (GCC) 13.2**
- **Git revision: v2.0.0-1542-gcd4e5db0**

This version includes updates for fixed headers and multiple defines.

## ✨ Features

- **🖥️ Dreamcast Hardware + OpenGL**: Optimized for SDL FAST BLIT SPEED.
- **⚡ 2D Blit Speed Benchmark**: Significant improvements in both software and hardware modes.
- **🔗 Integration with GLDC**: Utilizes Kazade's GLDC library for enhanced OpenGL compatibility and performance.

![Screenshot of SDL-DREAMHAL](https://github.com/ianmicheal/SDL-dreamhal--GLDC/blob/main/benchmark.png)

## 📊 Performance Comparison

Direct rendering update (no priming):

<table>
  <tr>
    <th>Resolution</th>
    <th>Mode</th>
    <th>Slow Points<br>(frames/sec)</th>
    <th>Fast Points<br>(frames/sec)</th>
    <th>Rect Fill<br>(rects/sec)</th>
    <th>32x32 Blits<br>(blits/sec)</th>
  </tr>
  <tr>
    <td rowspan="2">320x240</td>
    <td>Software</td>
    <td align="right">0.419749</td>
    <td align="right">36.0411</td>
    <td align="right">1314.51</td>
    <td align="right">3075.08</td>
  </tr>
  <tr>
    <td>Hardware</td>
    <td align="right">1.03212</td>
    <td align="right">62.699</td>
    <td align="right">2684.14</td>
    <td align="right">4133.2</td>
  </tr>
  <tr>
    <td rowspan="2">640x480</td>
    <td>Software</td>
    <td align="right">0.220216</td>
    <td align="right">20.444</td>
    <td align="right">709.51</td>
    <td align="right">3005.14</td>
  </tr>
  <tr>
    <td>Hardware</td>
    <td align="right">0.129902</td>
    <td align="right">15.7296</td>
    <td align="right">696.007</td>
    <td align="right">4043.44</td>
  </tr>
</table>

> 💡 This table compares performance across different resolutions and rendering methods.

## 🛠️ Dreamcast Integration

- **✅ Correct OpenGL integration**
- **🖼️ Textured video driver for virtual resolutions**
- **🖥️ Direct framebuffer video driver using store queues**
- **🖱️ Mouse emulation using analog pad**
- **🎮 Mapped keys as pad buttons**
- **🎨 Two texture internal color modes supported: RGB5551 and RGB4444**
- **🔊 Fast threaded audio driver**

## 🌐 SDL

SDL is a cross-platform multimedia library designed to provide low-level access to:
- Audio
- Keyboard
- Mouse
- Joystick
- 3D hardware via OpenGL
- 2D video framebuffer

## 🆕 New Features

- **🚄 Upgraded Blit Speed**: Utilizing moops dreamhal SH4 ASM memcpy and memset.
- **🔌 GLDC Integration**: Improved compatibility and performance with OpenGL 1.2.
# 🎮 SDL_dreamcast.h Functions Guide

## 📚 Table of Contents
- [Introduction](#-introduction)
- [Video Functions](#-video-functions)
- [Event Handling](#-event-handling)
- [Audio Functions](#-audio-functions)
- [Usage Example](#-usage-example)

## 🚀 Introduction

The `SDL_dreamcast.h` header file provides a set of functions as SDL add-ons specifically for the Dreamcast. These functions allow for fine-tuned control over video settings, input handling, and audio behavior on the Dreamcast hardware.

## 🖥️ Video Functions

### Setting the Video Driver

```c
SDL_DC_SetVideoDriver(SDL_DC_driver value)
```

Call this function before `SDL_Init` to choose the SDL video driver for Dreamcast.

| Driver | Description |
|:------:|-------------|
| `SDL_DC_DMA_VIDEO` (default) | Fastest video driver using double buffer. All graphic access uses RAM, and `SDL_Flip` sends data to VRAM using DMA. |
| `SDL_DC_TEXTURED_VIDEO` | Uses hardware texture for scaling, allowing virtual resolutions. PVR textures are always 2^n (128x128, 256x128, 512x256, etc.). |
| `SDL_DC_DIRECT_VIDEO` | Direct buffer video driver. Potentially faster than DMA driver when not using double buffering. |

### Other Video Functions

```c
SDL_DC_SetWindow(int width, int height)  // For textured video only
SDL_DC_VerticalWait(SDL_bool value)      // Enable/disable vertical retrace wait
SDL_DC_ShowAskHz(SDL_bool value)         // Enable/disable 50/60Hz choice (PAL only)
SDL_DC_Default60Hz(SDL_bool value)       // Set default to 60Hz (PAL only)
```

## 🕹️ Event Handling

### Mapping Dreamcast Buttons to SDL Keys

```c
SDL_DC_MapKey(int joy, SDL_DC_button button, SDLKey key)
```

Map a Dreamcast button to an `SDLKey`. 

| Parameter | Description |
|:---------:|-------------|
| `joy` | Dreamcast joystick port number (0, 1, 2, or 3) |
| `button` | Dreamcast button to map |
| `key` | SDL key to map to |

#### 📊 Default Button Mappings

<details>
<summary>Click to expand default mappings</summary>

| Button | Port 0 | Port 1 | Port 2 | Port 3 |
|:------:|:------:|:------:|:------:|:------:|
| SDL_DC_START | SDLK_RETURN | SDLK_z | SDLK_v | SDLK_m |
| SDL_DC_A | SDLK_LCTRL | SDLK_e | SDLK_y | SDLK_o |
| SDL_DC_B | SDLK_LALT | SDLK_q | SDLK_r | SDLK_u |
| SDL_DC_X | SDLK_SPACE | SDLK_x | SDLK_b | SDLK_COMMA |
| SDL_DC_Y | SDLK_LSHIFT | SDLK_c | SDLK_n | SDLK_PERIOD |
| SDL_DC_L | SDLK_TAB | SDLK_1 | SDLK_4 | SDLK_8 |
| SDL_DC_R | SDLK_BACKSPACE | SDLK_2 | SDLK_5 | SDLK_9 |
| SDL_DC_LEFT | SDLK_LEFT | SDLK_a | SDLK_f | SDLK_j |
| SDL_DC_RIGHT | SDLK_RIGHT | SDLK_d | SDLK_h | SDLK_l |
| SDL_DC_UP | SDLK_UP | SDLK_w | SDLK_t | SDLK_i |
| SDL_DC_DOWN | SDLK_DOWN | SDLK_s | SDLK_g | SDLK_k |

</details>

### Input Device Emulation

```c
SDL_DC_EmulateKeyboard(SDL_bool value)  // Enable/disable keyboard emulation
SDL_DC_EmulateMouse(SDL_bool value)     // Enable/disable mouse emulation
```

> ⚠️ **Note:** These functions require `SDL_OpenJoystick` to be called first.

## 🔊 Audio Functions

### Custom Sound Buffer Management

```c
SDL_DC_SetSoundBuffer(void *buffer)     // Set custom internal sound buffer
SDL_DC_RestoreSoundBuffer(void)         // Reset to default sound buffer
```

> 💡 **Tip:** Using a custom sound buffer can improve performance by avoiding memory copying in the sound callback.

## 💻 Usage Example

Here's an example demonstrating how to use some of these Dreamcast-specific functions:

```c
#include "include/SDL.h"
#include "include/SDL_dreamcast.h"

int main(int argc, char *argv[]) {
    SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_DC_VerticalWait(SDL_TRUE);
    SDL_DC_Default60Hz(SDL_TRUE);

    // Open joystick
    SDL_Joystick *joystick = SDL_JoystickOpen(0);
    if (joystick) {
        // Custom button mapping
        SDL_DC_MapKey(0, SDL_DC_A, SDLK_SPACE);
        
        // Enable mouse emulation
        SDL_DC_EmulateMouse(SDL_TRUE);
    }

    // Main game loop
    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }

        // Your game logic here...

        SDL_Flip(screen);
    }

    SDL_JoystickClose(joystick);
    SDL_Quit();
    return 0;
}
```

This example demonstrates:
- Setting up the video mode with the DMA driver
- Opening a joystick and customizing button mapping
- Enabling mouse emulation
- A basic game loop with event handling

> 🛠️ **Remember:** Compile your code with the appropriate SDL libraries and Dreamcast-specific settings.

---

📌 For more information and advanced usage, refer to the official SDL documentation and Dreamcast development resources.
## Additional Resources

- **PORTED TO SDL + OpenGL Dreamhal + GLDC Demos**:
- **SDL[DREAMHAL]+[GLDC]KOS2.0 TWINKLE STAR DEMO HARDWARE**:
  - [Demo 1](https://youtu.be/aTKOW4GrLsQ)
  -  ** SDL[DREAMHAL]+[GLDC]KOS2.0 BLENDING GLASS CUBE HARDWARE**:
  - [Demo 2](https://youtu.be/GM4JnKxp1ZQ)
  - ** SDL[DREAMHAL]+[GLDC]KOS2.0 SPINNING TEXTURES CUBE 60FPS
  - [Demo 3](https://youtu.be/G0KFthogrPg)
  - **SDLQUAKE DREAMHAL**:
  - [Demo port quake 1](https://youtu.be/QiEo3FNNEZc).
  - **OLD VERSION CANT BE COMPILED OR USED ON KOS2.0**
   - **Old Version**: [SDL by Chui](http://chui.dcemu.co.uk/sdl.html)

---

For a complete list of features and detailed documentation, please visit the [GLDC repository](https://gitlab.com/simulant/GLdc).

## Credits
- **BERO**: Original work  for SDL on Dreamcast.
- **Chui**: Original work and improvements for SDL on Dreamcast.
- **Kazade**: GLDC library and enhancements.
- **MRNEO240**:GLDC Optimizing and Support.
- **GPF**: For fixing compile errors and warnings in new kos and gcc13.2/14
GNU Lesser General Public License, version 2.1
https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html
# DEPRECATED

The 1.2 branch of SDL is deprecated. While we occasionally collect fixes
in revision control, there has not been a formal release since 2012, and
we have no intention to do future releases, either.

Current development is happening in SDL 2.0.x, which gets regular
releases and can be found at:

https://github.com/libsdl-org/SDL

Thanks!
# Simple DirectMedia Layer (SDL) Version 1.2

https://www.libsdl.org/

This is the Simple DirectMedia Layer, a general API that provides low
level access to audio, keyboard, mouse, joystick, 3D hardware via OpenGL,
and 2D framebuffer across multiple platforms.

The current version supports Linux, Windows CE/95/98/ME/XP/Vista, BeOS,
MacOS Classic, Mac OS X, FreeBSD, NetBSD, OpenBSD, BSD/OS, Solaris, IRIX,
and QNX.  The code contains support for Dreamcast, Atari, AIX, OSF/Tru64,
RISC OS, SymbianOS, Nintendo DS, and OS/2, but these are not officially
supported.

SDL is written in C, but works with C++ natively, and has bindings to
several other languages, including Ada, C#, Eiffel, Erlang, Euphoria,
Guile, Haskell, Java, Lisp, Lua, ML, Objective C, Pascal, Perl, PHP,
Pike, Pliant, Python, Ruby, and Smalltalk.

This library is distributed under GNU LGPL version 2, which can be
found in the file  "COPYING".  This license allows you to use SDL
freely in commercial programs as long as you link with the dynamic
library.

The best way to learn how to use SDL is to check out the header files in
the "include" subdirectory and the programs in the "test" subdirectory.
The header files and test programs are well commented and always up to date.
More documentation is available in HTML format in "docs/index.html".

The test programs in the "test" subdirectory are in the public domain.

Enjoy!

Sam Lantinga (slouken@libsdl.org)



<p align="center">
  <img src="https://www.libsdl.org/media/SDL_logo.png" alt="SDL Logo" width="200"/>
</p>

<h1 align="center">SDL_gfx Library</h1>

<p align="center">
  <strong>SDL graphics drawing primitives and other support functions</strong>
</p>

<p align="center">
  <a href="#about">About</a> •
  <a href="#components">Components</a> •
  <a href="#features">Features</a> •
  <a href="#usage">Usage</a> •
  <a href="#contributors">Contributors</a>
</p>

## About

The SDL_gfx library evolved out of two powerful components:

1. **SDL_gfxPrimitives**: Provided basic drawing routines such as lines, circles, and polygons.
2. **SDL_rotozoom**: Implemented an interpolating rotozoomer for SDL surfaces.

Together, they form a comprehensive graphics library for SDL developers.

## Components

The SDL_gfx library consists of four main components:

| Component | Header File | Description |
|-----------|-------------|-------------|
| 🎨 Graphic Primitives | `SDL_gfxPrimitves.h` | Basic drawing functions |
| 🔄 Rotozoomer | `SDL_rotozoom.h` | Image rotation and zooming |
| ⏱️ Framerate control | `SDL_framerate.h` | FPS management |
| 🖼️ MMX image filters | `SDL_imageFilter.h` | Fast image processing |

## Features

- ✅ Backwards compatible with original SDL_gfxPrimitives and SDL_rotozoom
- 🔧 Written in plain C for maximum compatibility
- 🔀 Can be seamlessly used in C++ projects

## Usage

To harness the power of SDL_gfx in your project, include these headers:

```c
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>
#include <SDL_framerate.h>
#include <SDL_imageFilter.h>
```

## Contributors

We're grateful to the following contributors who have helped improve SDL_gfx:

- **Ingo van Lil** (inguin at gmx.de) - Fixed filledbox
- **Pete Shinners** (pete at shinners.org) - Non-alpha line drawing code
- **Karl Bartel** (karlb at gmx.net) - Various fixes
- **Danny van Bruggen** (danny at froukepc.dhs.org) - Testing, suggestions, and VisualC makefile
- **Stephane Magnenat** (nct at wg0.ysagoon.com) - AA-circle/-ellipse code idea
- **Anders Lindström** (cal at swipnet.se) - Faster blending routines
- **James Turk** (jturk at conceptofzero.com) - VisualC7 project file
- **Thomas Tongue** (TTongue at imagiware.com) - Project Builder package
- **Kentaro Fukuchi** (fukuchi at is.titech.ac.jp) - Fix for filledPolygon
- **Mike Gorchak** (mike at malva.ua) - QNX6 patch
- **Eike Lange** (eike.lange at uni-essen.de) - Pie idea
- **Todor Prokopov** (koprok at dir.bg) - Dynamic font setup
- **Victor (Haypo) Stinner** (victor.stinner at haypocalc.com) - Horizontal/Vertical flipping code
- **Michael Wybrow** (mjwybrow at cs.mu.oz.au) - OSX build fixes
- **Dries Verachtert** (dries at ulyssis.org) - gcc3.4 build fixes

<hr>

<p align="center">
  Made with ❤️ by the SDL community
</p>

<p align="center">
  <a href="https://www.libsdl.org/">Visit SDL.org</a> •
  <a href="https://github.com/libsdl-org/SDL">SDL on GitHub</a>
</p>

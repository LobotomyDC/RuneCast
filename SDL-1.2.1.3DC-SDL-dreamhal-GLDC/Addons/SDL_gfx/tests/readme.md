# SDL_gfx Library

<p align="center">
  <img src="https://www.libsdl.org/media/SDL_logo.png" alt="SDL Logo" width="200"/>
</p>

<p align="center">
  <strong>SDL graphics drawing primitives and other support functions</strong>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#graphics-primitives">Graphics Primitives</a> •
  <a href="#rotozoomer">Rotozoomer</a> •
  <a href="#framerate-functions">Framerate Functions</a> •
  <a href="#usage">Usage</a>
</p>

## Features

- ✅ Fully alpha-aware primitives
- 🎨 Supports surface depths of 1, 2, 3, and 4 bytes per pixel
- 🚀 Works well with hardware accelerated surfaces
- 🖌️ Anti-Aliased drawing primitives available

## Graphics Primitives

SDL_gfx provides a wide range of drawing primitives. All color routines expect the color to be in format `0xRRGGBBAA`.

### Available Primitives

- Pixel
- Horizontal line
- Vertical line
- Rectangle
- Filled rectangle (Box)
- Line
- AA Line
- Circle
- AA Circle
- Filled Circle
- Ellipse
- AA Ellipse
- Filled Ellipse
- Pie
- Filled Pie
- Trigon
- AA-Trigon
- Filled Trigon
- Polygon
- AA-Polygon
- Filled Polygon
- Bezier Curve
- 8x8 Characters/Strings

### Example Function Signatures

```c
int pixelColor(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color);
int lineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, 
              Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int filledCircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
```

## Rotozoomer

The rotozoomer provides functions for rotating and zooming surfaces.

### Key Functions

```c
SDL_Surface * rotozoomSurface (SDL_Surface *src, double angle, double zoom, int smooth);
SDL_Surface * rotozoomSurfaceXY (SDL_Surface *src, double angle, double zoomx, double zoomy, int smooth);
SDL_Surface * zoomSurface (SDL_Surface *src, double zoomx, double zoomy, int smooth);
```

### Features

- 🔄 Rotate and zoom 32bit or 8bit surfaces
- 🖼️ Creates a new destination surface
- 🔍 Optional anti-aliasing for 32bit surfaces
- 🔁 Supports negative zoom for axis flipping (rotozoomSurfaceXY)

## Framerate Functions

These functions help maintain a constant framerate in your application.

### Key Functions

```c
void SDL_initFramerate(FPSmanager * manager);
int SDL_setFramerate(FPSmanager * manager, int rate);
int SDL_getFramerate(FPSmanager * manager);
void SDL_framerateDelay(FPSmanager * manager);
```

### Features

- ⏱️ More accurate than simple delay-based timing
- 📊 Keeps track of desired game time per frame
- 🎯 Aims for precise long-term timing (e.g., 100th frame at exactly 2.00sec for 50Hz)

## Usage

Include the necessary headers in your project:

```c
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>
#include <SDL_framerate.h>
```

For detailed usage instructions and more examples, please refer to the official SDL_gfx documentation.

---

<p align="center">
  Made with ❤️ by the SDL community
</p>

<p align="center">
  <a href="https://www.libsdl.org/">Visit SDL.org</a> •
  <a href="https://github.com/libsdl-org/SDL">SDL on GitHub</a>
</p>

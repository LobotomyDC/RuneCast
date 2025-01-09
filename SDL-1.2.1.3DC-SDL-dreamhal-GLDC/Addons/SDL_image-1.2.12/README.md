<h1 align="center">
  <img src="https://www.libsdl.org/media/SDL_logo.png" alt="SDL Logo" width="200"/>
</p>
  SDL_image 1.2.12
  <br>
</h1>

<h4 align="center">A simple library to load images of various formats as SDL surfaces</h4>

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#supported-formats">Supported Formats</a> •
  <a href="#how-to-use">How To Use</a> •
  <a href="#api">API</a> •
  <a href="#dependencies">Dependencies</a> •
  <a href="#build-instructions">Build Instructions</a> •
  <a href="#license">License</a>
</p>

<p align="center">
  <img src="https://your-image-url-here.com/demo.gif" alt="SDL_image demo">
</p>

## Key Features

* Load images of various formats as SDL surfaces 🖼️
* Simple and easy-to-use API 🛠️
* Support for multiple image formats 📊
* Cross-platform compatibility 🖥️

## Supported Formats

SDL_image supports a wide range of image formats:

| Format | Description |
| ------ | ----------- |
| BMP    | Windows Bitmap |
| PNM    | Portable Anymap (PBM/PGM/PPM) |
| XPM    | X11 Pixmap |
| LBM    | Interleaved Bitmap |
| PCX    | ZSoft PC Paintbrush |
| GIF    | Graphics Interchange Format |
| JPEG   | Joint Photographic Experts Group |
| PNG    | Portable Network Graphics |
| TGA    | Truevision TARGA |
| TIFF   | Tagged Image File Format |

## How To Use

To clone and run this application, you'll need [Git](https://git-scm.com) and a C compiler installed on your computer. From your command line:

```bash
# Clone this repository
$ git clone https://github.com/yourusername/SDL_image.git

# Go into the repository
$ cd SDL_image

# Compile the library
$ ./configure
$ make

# Install the library
$ sudo make install
```

## API

Include the following header in your code:

```c
#include "SDL_image.h"
```

### Loading Images

SDL_image provides three main functions for loading images:

1. **Load from file:**
   ```c
   SDL_Surface *IMG_Load(const char *file);
   ```

2. **Load from SDL_RWops:**
   ```c
   SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc);
   ```

3. **Load from SDL_RWops with specified image type:**
   ```c
   SDL_Surface *IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type);
   ```
   Where `type` is a string specifying the format (e.g., "PNG" or "pcx").

> **Note:** `IMG_Load_RW` cannot load TGA images.

### Creating Surface from XPM

To create a surface from an XPM image included in C source:

```c
SDL_Surface *IMG_ReadXPMFromArray(char **xpm);
```

## Dependencies

SDL_image relies on several libraries for full functionality:

- JPEG support: [JPEG library](http://www.ijg.org/)
- PNG support: [PNG library](http://www.libpng.org/pub/png/libpng.html) and [Zlib library](http://www.gzip.org/zlib/)
- TIFF support: [TIFF library](http://www.libtiff.org/)

## Build Instructions

If you have the dependencies installed in non-standard locations, you can specify the paths during configuration:

```sh
sh ./configure CPPFLAGS="-I/path/to/include" LDFLAGS="-L/path/to/lib"
```

After successful configuration, build and install the library:

```sh
make
sudo make install
```

> 💡 You may need to add `/path/to/lib` to your `LD_LIBRARY_PATH` for correct shared library loading.

## License

This project is licensed under the zlib License - see the [COPYING](COPYING) file for details.

---

<p align="center">
  Made with ❤️ by the SDL community
</p>

<p align="center">
  <a href="https://discourse.libsdl.org/">Forums</a> •
  <a href="https://discord.gg/SDL">Discord</a> •
  <a href="https://github.com/libsdl-org/SDL_image/issues">Issue Tracker</a>
</p>


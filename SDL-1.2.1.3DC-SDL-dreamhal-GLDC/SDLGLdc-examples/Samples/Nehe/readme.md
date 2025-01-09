
# 🎮 SDL-1.2.1.3DC: SDL+OpenGL for Dreamcast

## 📚 Table of Contents
- [Introduction](#-introduction)
- [Features](#-features)
- [Project Structure](#-project-structure)
- [NeHe Lessons](#-nehe-lessons)
- [Getting Started](#-getting-started)
- [Contributing](#-contributing)
- [Resources](#-resources)

## 🌟 Introduction

SDL-1.2.1.3DC is a powerful port of SDL 1.2.13 tailored specifically for the Sega Dreamcast console, integrated with OpenGL support. This project brings the flexibility of SDL and the 3D capabilities of OpenGL to Dreamcast development, allowing for creation of both 2D and 3D games and applications.

## ✨ Features

- 🖥️ Full SDL 1.2.13 implementation for Dreamcast
- 🔺 OpenGL support for 3D graphics
- 🕹️ Dreamcast controller support
- 🎨 Hardware-accelerated 2D and 3D rendering
- 🚀 Optimized for Dreamcast's SH-4 processor
- 📚 Includes NeHe OpenGL Lessons adapted for Dreamcast

## 📁 Project Structure

```
### Available Lessons:

- 📁 **01**: Setting up an OpenGL Window
- 📁 **02**: Your First Polygon
- 📁 **03**: Adding Color
- 📁 **04**: Rotation
- 📁 **05**: 3D Shapes
- 📁 **06**: Texture Mapping
- 📁 **07**: Texture Filters, Lighting & Keyboard Control
- 📁 **08**: Blending

```

- `SDLGLdc-examples/`: Contains example projects and samples
- `Samples/Nehe/`: NeHe OpenGL Lessons adapted for Dreamcast
- `src/`: Source code for SDL-1.2.1.3DC
- `include/`: Header files


## 📘 NeHe Lessons

This project includes adaptations of the famous NeHe OpenGL Lessons, optimized for the Dreamcast hardware. These lessons cover a wide range of OpenGL topics, from basic shape rendering to advanced techniques like texture mapping and lighting.All ported from Opengl+SDL by Ian micheal

Included lessons:
- 📐 Lesson 01: Setting up an OpenGL Window
- 🔷 Lesson 02: Creating Polygons
- 🌈 Lesson 03: Adding Color
- 🔄 Lesson 04: Rotation
- 🧊 Lesson 05: 3D Shapes
- 🖼️ Lesson 06: Texture Mapping
- 🔦 Lesson 07: Texture Filters, Lighting & Keyboard Control
- 🌓 Lesson 08: Blending
- 

Each lesson is contained in its own directory under `SDLGLdc-examples/Samples/Nehe/`, with source code and assets ready to compile and run on the Dreamcast.

## 🚀 Getting Started

You have to build and compile sdl for both framebuffer and opengl version before you do this.

1. Clone the repository:
   ```
   git clone https://github.com/your-repo/SDL-1.2.1.3DC.git
   ```
2. Install the Dreamcast SDK and toolchain
3. Navigate to an example project:
   ```
   cd SDL-1.2.1.3DC/SDLGLdc-examples/Samples/Nehe/01
   ```
4. Compile and build for Dreamcast
5. Run the resulting binary on Dreamcast hardware or an emulator



## 🤝 Contributing

Contributions to SDL-1.2.1.3DC are welcome! Whether it's bug fixes, new features, or additional examples, your input is valuable. Please read our contributing guidelines in `CONTRIBUTING.md` before submitting pull requests.

## 📚 Resources

- [SDL Documentation](https://www.libsdl.org/release/SDL-1.2.15/docs/)
- [OpenGL Red Book](http://www.opengl-redbook.com/)
- [Dreamcast Programming](http://mc.pp.se/dc/)
- [NeHe Productions](http://nehe.gamedev.net/)

---

📌 **Note**: This project is a community effort and is not officially affiliated with Sega, SDL, or NeHe Productions.

🎉 Happy Dreamcast development with SDL and OpenGL!

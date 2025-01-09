
# 🎮 Dreamcast SDL1.2.1.3 Ports

![Dreamcast](https://img.shields.io/badge/Dreamcast-20B2AA?style=for-the-badge&logo=sega&logoColor=white)
![SDL](https://img.shields.io/badge/SDL-1.2.1.3-blue?style=for-the-badge&logo=sdl&logoColor=white)

This repository contains ports of classic SDL examples from PC to the Dreamcast, utilizing SDL1.2.1.3. These ports showcase the proper use of Dreamcast-specific functions and optimizations.

## 🚀 Features

- ✅ Dreamcast-optimized SDL1.2.1.3 implementation
- 🖥️ Custom video driver setup for optimal performance
- 🎮 Controller to keyboard/mouse mapping
- 🚪 Proper exit handling back to Dreamcast loader

## 🛠️ Dreamcast-Specific Functions

### Video Driver Setup

```c
SDL_DC_ShowAskHz(SDL_FALSE);
SDL_DC_Default60Hz(SDL_FALSE);
SDL_DC_VerticalWait(SDL_FALSE);
SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
```

### Controller Mapping

```c
SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);
SDL_JoystickEventState(SDL_ENABLE);
joystick = SDL_JoystickOpen(0);
```

### Proper Exit Handling

```c
if (event.jbutton.button == SDL_DC_START) {
    // Exit code here
}
```

## 📋 Examples

1. 🌀 **Warp Effect**: A mesmerizing image warping demo


## 🚀 Getting Started

1. Clone this repository
2. Set up your Dreamcast development environment
3. Compile the examples using your Dreamcast SDK
4. Run the resulting binaries on your Dreamcast or emulator

## 📚 Documentation

For more detailed information on Dreamcast-specific SDL functions, please refer to the [Dreamcast SDL1.2.1.3 Documentation](link-to-documentation).

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 🙏 Acknowledgments

- SDL development team
- Dreamcast homebrew community
- Original PC SDL example creators

---

⭐ Star this repository if you find it helpful!

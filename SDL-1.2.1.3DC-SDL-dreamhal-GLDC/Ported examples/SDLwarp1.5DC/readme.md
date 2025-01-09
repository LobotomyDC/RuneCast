
# 🎮 Dreamcast SDL Warp Example

![Dreamcast](https://img.shields.io/badge/Dreamcast-20B2AA?style=for-the-badge&logo=sega&logoColor=white)
![SDL](https://img.shields.io/badge/SDL-1.2.13DC-blue?style=for-the-badge&logo=sdl&logoColor=white)
![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)

A mesmerizing image warping demo showcasing the power of SDL on the Dreamcast! 🌀✨

## 🚀 Features

- 🖼️ Multi-image support with smooth transitions
- 🕹️ Interactive warp control using D-pad
- 🔄 Fullscreen display with automatic image scaling
- ⚡ Optimized for Dreamcast hardware

## 🛠️ Technology Stack

- **SDL Version**: Custom SDL1.2.13DC for Dreamcast
- **Platform**: Sega Dreamcast
- **Language**: C

## 🧠 How It Works

1. 🎨 **Image Loading**: Loads multiple images from the Dreamcast's romdisk.
2. 🖥️ **Screen Setup**: Utilizes Dreamcast-specific video modes for optimal performance.
3. 🔄 **Image Conversion**: Scales and converts images to match the screen format.
4. 🌀 **Warp Effect**: Applies a real-time warping effect to the displayed image.
5. 🎮 **User Interaction**: 
   - 🅰️ A Button: Cycle through loaded images
   - 🕹️ D-Pad: Adjust warp intensity
   - 🏁 Start Button: Exit the program
  
   - # 🌀 Understanding the Warp Effect

The warp effect in this example creates a dynamic, liquid-like distortion of the image. Here's how it's implemented:

## 1. Initialization

### Warp Structure
The effect uses a `struct warp` which contains:
- Source image (`src`)
- Destination surface (`dst`)
- Frame buffer (`framebuf`)
- Offset table (`offstable`)
- Distance table (`disttable`)
- Sine table (`sintable`)
- Color table (`ctable`)

### Tables Setup
- **Sine Table**: Pre-calculated sine values for quick lookup.
- **Offset Table**: Stores pointers to each row of the source image for fast access.
- **Distance Table**: Pre-calculated distances from the center of the image.

## 2. Main Loop Calculations

In each frame, the program calculates warp parameters:

```c
xw = (int)(sin((tval+100)*M_PI/128) * 30 * warp_intensity);
yw = (int)(sin((tval)*M_PI/256) * -35 * warp_intensity);
cw = (int)(sin((tval-70)*M_PI/64) * 50 * warp_intensity);
xw += (int)(sin((tval-10)*M_PI/512) * 40 * warp_intensity);
yw += (int)(sin((tval+30)*M_PI/512) * 40 * warp_intensity);
```

These create the dynamic motion of the warp effect. `warp_intensity` allows user control over the strength of the effect.

## 3. Warp Function

The core of the effect is in the `doWarp` functions (e.g., `doWarp16bpp`). Here's what happens:

1. **Setup Color Table**: 
   - Creates a table of x and y offsets based on sine waves.

2. **Pixel Mapping**:
   - For each pixel in the destination:
     a. Calculate its position in the source image using the distance and color tables.
     b. Apply bounds checking to ensure we're within the image.
     c. Copy the color from the source to the destination.

3. **Mathematical Basis**:
   - The warp effect is essentially a 2D displacement map.
   - Each pixel's new position is determined by:
     ```
     new_x = x + displacement_x
     new_y = y + displacement_y
     ```
   - Where `displacement_x` and `displacement_y` are derived from sine waves and the distance from the center.

## 4. Performance Optimizations

- **Pre-calculated Tables**: Sine and distance tables avoid costly real-time calculations.
- **Pointer Arithmetic**: Direct memory access for faster pixel manipulation.
- **Dreamcast-specific**: Using `SDL_DC_DMA_VIDEO` for optimal video performance.

## 5. User Interaction

- D-pad controls `warp_intensity`, allowing real-time adjustment of the effect's strength.
- This modifies the amplitude of the sine waves used in the displacement calculations.

## 6. Multi-Image Support

- The warp structure is reinitialized when switching images, allowing the effect to adapt to different image sizes and formats.

By combining these elements, the program creates a smooth, dynamic warping effect that distorts the image in real-time, creating the illusion of a liquid-like surface

## 💡 Dreamcast-Specific Optimizations

- Uses `SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO)` for fast rendering.
- Implements `SDL_DC_ShowAskHz` and `SDL_DC_Default60Hz` for display tuning.
- Utilizes `SDL_DC_MapKey` to map Dreamcast controller buttons to keyboard inputs.

## 🚀 Getting Started

1. Set up your Dreamcast development environment.
2. Clone this repository.
3. Compile using your Dreamcast toolchain:
   ```
   type make 
   ```
4. Run on Dreamcast hardware or an emulator.

## 📚 Code Highlights

```c
SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF);
```
Setting up Dreamcast-optimized video mode.

```c
SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);
```
Mapping Dreamcast START button to ESC key for easy porting.

## 🤝 Contributing

Contributions are welcome! Feel free to submit pull requests or open issues.

## 📄 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Original Warp-SDL by Emmanuel Marty
- SDL port by Sam Lantinga
- Dreamcast port and enhancements by Ian Micheal

---

⭐ Star this repo if you find it interesting!

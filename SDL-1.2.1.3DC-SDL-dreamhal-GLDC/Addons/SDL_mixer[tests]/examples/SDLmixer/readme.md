
# 🎮 SDL Mixer Test SDL 1.2.1.3DC[DREAMHAL] SDL Joystick & Animation Renderer

![SDL](https://img.shields.io/badge/Library-SDL1.2-orange.svg)
![C](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Dreamcast-green.svg)

This project is an example of using **SDL 1.2.1.3DC[DREAMHAL]** for joystick input and sprite animation on the Dreamcast. It handles multiple input methods (analog stick, D-pad buttons), integrates audio playback using **SDL_mixer**, and renders sprites with smooth transitions. Built with Dreamcast support via KallistiOS, this project demonstrates the use of analog and digital controls in a gaming environment.

### ✍️ Author

**Ian Micheal**

## 🚀 Features

- **Analog Stick & D-pad Input**: 
  - Full joystick handling with analog stick movement and D-pad directional controls.
  - Implements dead zones for more precise analog control.
  
- **Smooth Sprite Animation**:
  - Renders animated frames, smoothly transitioning between them.
  - Supports clamping player position within the screen boundaries.

- **Audio Playback**:
  - Integrated background music and sound effects using **SDL_mixer**.
  - Different sounds triggered based on button presses (A, B, X, Y).

- **Joystick Input Debugging**:
  - Outputs real-time debug information for analog stick positions (X and Y axes) and player movement to help with testing.

## 📋 How It Works

- **Analog Stick**: 
  - Reads input from the joystick, applying dead zones to avoid small drift movements.
  - Normalizes the input and scales it to move the player smoothly across the screen.
  
- **D-pad**: 
  - Reads digital input from the D-pad, allowing for precise movement in four directions.
  
- **Audio**:
  - Pressing the A, B, X, and Y buttons plays different sound effects or background music.

- **Frame Clamping**:
  - Ensures that the player's character remains within the boundaries of the screen.

## 🛠️ Requirements

- **KallistiOS** (for Dreamcast builds)
- **SDL 1.2.1.3** and **SDL_mixer1.2.6**
- Compatible joystick or gamepad for testing input
-  https://www.linux-games.com/sfont-fonts/


## 🎨 Screenshots


![Joystick Input](https://img.shields.io/badge/Joystick-Input-blue.svg)
![Animation](https://img.shields.io/badge/Sprites-Animated-red.svg)


# 🎮 OpenGL Texture Filters, Lighting & Keyboard Control Tutorial

## 📚 Table of Contents
- [Introduction](#-introduction)
- [Key Concepts](#-key-concepts)
- [Implementation](#-implementation)
- [Controls](#-controls)
- [Code Breakdown](#-code-breakdown)
- [Tips and Tricks](#-tips-and-tricks)
- [Conclusion](#-conclusion)

Here's a visual example of the Texture Filters, Lighting & Keyboard Control Tutorial achieved in this implementation:

![OpenGL Blending Example](https://github.com/ianmicheal/SDL-1.2.1.3DC/raw/SDL-dreamhal--GLDC/SDLGLdc-examples/Samples/Nehe/07/07.png)
## 🌟 Introduction

This tutorial covers advanced OpenGL techniques including texture filtering, basic lighting, and keyboard controls for interactive 3D graphics. You'll learn how to create a rotating textured cube with dynamic lighting and user interaction.

## 🔑 Key Concepts

1. **Texture Filtering**: 
   - 🔹 GL_NEAREST (no filtering)
   - 🔹 GL_LINEAR (linear filtering)
   - 🔹 Mipmapping

2. **Lighting**:
   - 💡 Ambient light
   - 🔦 Diffuse light
   - 📐 Normal vectors

3. **User Interaction**:
   - ⌨️ Keyboard controls for rotation and movement
   - 🔀 Texture filter switching

## 🛠️ Implementation

### Texture Loading and Filtering
```c
int LoadGLTextures() {
    // ... Load bitmap
    glGenTextures(3, &texture[0]);
    
    // GL_NEAREST filtering
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // GL_LINEAR filtering
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Mipmapped texture
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // ... Create textures
}
```

### Lighting Setup
```c
GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
glEnable(GL_LIGHT1);
```

## 🕹️ Controls

| Key       | Action                     |
|-----------|----------------------------|
| L         | Toggle lighting on/off     |
| F         | Cycle through texture filters |
| Page Up   | Move object into screen    |
| Page Down | Move object out of screen  |
| Arrow Keys| Control rotation speed     |

## 💻 Code Breakdown

1. **Texture Loading**: 
   - Load bitmap image
   - Create three textures with different filtering methods

2. **Lighting Setup**:
   - Define ambient and diffuse light properties
   - Set light position
   - Enable lighting

3. **Drawing the Cube**:
   - Use `glNormal3f` to define face normals for proper lighting
   - Apply textures to each face

4. **User Input Handling**:
   - Check for key presses in the main loop
   - Update object position and rotation based on input

## 💡 Tips and Tricks

- 🎨 Experiment with different texture sizes to see the effect of different filters
- 🔍 Observe how lighting changes the appearance of the textured surfaces
- 🔄 Try modifying the light position for different effects
- 🎛️ Adjust rotation and movement speeds for smoother or more dramatic animations

## 🏁 Conclusion

By the end of this tutorial, you should be comfortable with:
- Implementing different texture filtering techniques
- Setting up basic lighting in OpenGL
- Handling keyboard input for interactive 3D graphics

Keep experimenting and building on these concepts to create more complex and engaging OpenGL applications!

---

📌 **Note**: This tutorial is based on legacy OpenGL. Modern OpenGL uses shader-based approaches, but these fundamental concepts remain relevant.

🔗 For more OpenGL tutorials and resources, visit [NeHe Productions](http://nehe.gamedev.net/).

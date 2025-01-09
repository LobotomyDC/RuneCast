
# 🔺 Your First OpenGL Polygon

## 📚 Table of Contents
- [Introduction](#-introduction)
- [Key Concepts](#-key-concepts)
- [Drawing Shapes](#-drawing-shapes)
- [Code Breakdown](#-code-breakdown)
- [Tips and Tricks](#-tips-and-tricks)
- [Conclusion](#-conclusion)
Here's a visual example of the OpenGL 3D  First OpenGL Polygon Tutorial achieved in this implementation:

![OpenGL First OpenGL Polygon](https://github.com/ianmicheal/SDL-1.2.1.3DC/raw/SDL-dreamhal--GLDC/SDLGLdc-examples/Samples/Nehe/02/02.png)
## 🌟 Introduction

This tutorial builds upon the basics of creating an OpenGL window, introducing you to drawing fundamental shapes: triangles and quads. You'll learn how to create and position these shapes in 3D space.

## 🔑 Key Concepts

1. **Coordinate System**: 
   - X-axis: Left (-) to Right (+)
   - Y-axis: Bottom (-) to Top (+)
   - Z-axis: Into screen (-) to Towards viewer (+)

2. **Basic Commands**:
   - `glLoadIdentity()`: Reset to center of screen
   - `glTranslatef(x, y, z)`: Move drawing position
   - `glBegin(mode)` & `glEnd()`: Define shape boundaries

3. **Drawing Modes**:
   - `GL_TRIANGLES`: For 3-point shapes
   - `GL_QUADS`: For 4-point shapes
   - `GL_POLYGON`: For shapes with more points

## 🎨 Drawing Shapes

### Triangle
```c
glBegin(GL_TRIANGLES);
    glVertex3f( 0.0f, 1.0f, 0.0f);  // Top
    glVertex3f(-1.0f,-1.0f, 0.0f);  // Bottom Left
    glVertex3f( 1.0f,-1.0f, 0.0f);  // Bottom Right
glEnd();
```

### Square (Quad)
```c
glBegin(GL_QUADS);
    glVertex3f(-1.0f, 1.0f, 0.0f);  // Top Left
    glVertex3f( 1.0f, 1.0f, 0.0f);  // Top Right
    glVertex3f( 1.0f,-1.0f, 0.0f);  // Bottom Right
    glVertex3f(-1.0f,-1.0f, 0.0f);  // Bottom Left
glEnd();
```

## 💻 Code Breakdown

1. **Setting Up the Scene**:
   ```c
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity();
   glTranslatef(-1.5f, 0.0f, -6.0f);
   ```

2. **Drawing the Triangle**:
   - Use `GL_TRIANGLES` mode
   - Define 3 vertices

3. **Moving to Draw Square**:
   ```c
   glTranslatef(3.0f, 0.0f, 0.0f);
   ```

4. **Drawing the Square**:
   - Use `GL_QUADS` mode
   - Define 4 vertices in clockwise order

## 💡 Tips and Tricks

- 🔄 Clockwise vertex order creates back-facing polygons
- 🔁 Counter-clockwise order creates front-facing polygons
- 🔢 Group multiple shapes of the same type within one `glBegin()`/`glEnd()` pair
- 📏 OpenGL units are arbitrary - maintain consistency in your chosen scale

## 🏁 Conclusion

By mastering these basics, you've taken the first step in OpenGL graphics programming. You can now:
- Create simple 2D shapes in a 3D space
- Position these shapes using translations
- Understand the fundamental structure of OpenGL drawing commands

Next steps:
- Experiment with different shapes and positions
- Try adding color to your shapes
- Explore rotation and scaling transformations

---

📌 **Note**: This tutorial uses immediate mode rendering, which is simple but deprecated in modern OpenGL. However, these concepts form the foundation for understanding more advanced techniques.

🔗 For more OpenGL tutorials and resources, visit [NeHe Productions](http://nehe.gamedev.net/).

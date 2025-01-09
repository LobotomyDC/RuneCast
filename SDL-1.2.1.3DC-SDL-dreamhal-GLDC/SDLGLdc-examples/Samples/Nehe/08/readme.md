# OpenGL Blending in SDL-1.2.1.3DC

This README explains the concepts of blending in OpenGL, as demonstrated in the SDL-1.2.1.3DC project for the Dreamcast.

## Example Implementation

You can find the full source code and implementation details for this OpenGL blending example here:
[SDL-1.2.1.3DC Nehe Lesson 08](https://github.com/ianmicheal/SDL-1.2.1.3DC/tree/SDL-dreamhal--GLDC/SDLGLdc-examples/Samples/Nehe/08)

## Visual Demonstration

Here's a visual example of the blending effects achieved in this implementation:

![OpenGL Blending Example](https://github.com/ianmicheal/SDL-1.2.1.3DC/raw/SDL-dreamhal--GLDC/SDLGLdc-examples/Samples/Nehe/08/08.png)

This image demonstrates a transparent cube with colorful, blended internal elements, showcasing the power of OpenGL blending techniques.

## Understanding Blending

Blending in OpenGL allows us to create transparent and translucent effects by combining the colors of overlapping objects. Here's how it works:

### Simple Transparency

- Alpha is used as a fourth color component (RGBA instead of RGB).
- Alpha values range from 0.0 (fully transparent) to 1.0 (fully opaque).
- Use `glColor4f()` to specify colors with alpha.

### The Blending Equation

For the mathematically inclined, the basic blending equation is:

```
(Rs Sr + Rd Dr, Gs Sg + Gd Dg, Bs Sb + Bd Db, As Sa + Ad Da)
```

Where `s` and `d` represent source and destination pixels, and `S` and `D` are blend factors.

### Implementing Blending

1. Enable blending with `glEnable(GL_BLEND)`.
2. Set the blend function with `glBlendFunc()`.
3. Draw transparent objects after opaque ones.
4. Sort transparent objects from back to front for correct rendering.

### Texture Mapping and Blending

When using textured objects:
- Texture colors are multiplied by the current color in modulated texture mode.
- Alpha can be read from texture maps using `GL_RGBA` format.

## Best Practices

- Draw transparent objects after opaque ones.
- Sort transparent objects by depth (farthest first).
- Be mindful of artifacts that can occur with certain blending techniques.

## Conclusion

OpenGL blending is a powerful technique for creating realistic transparent and translucent effects. By understanding and properly implementing these concepts, you can achieve sophisticated visual results in your OpenGL applications, as demonstrated in the SDL-1.2.1.3DC project for the Dreamcast.

For more details and the full implementation, check out the [source code](https://github.com/ianmicheal/SDL-1.2.1.3DC/tree/SDL-dreamhal--GLDC/SDLGLdc-examples/Samples/Nehe/08) in the GitHub repository.

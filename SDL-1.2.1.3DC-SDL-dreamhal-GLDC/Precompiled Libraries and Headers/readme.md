
# 🎮 Dreamcast Development with KallistiOS and SDL
![KallistiOS Version](https://img.shields.io/badge/KallistiOS-v2.1.0-blue)
![SDL Version](https://img.shields.io/badge/SDL-1.2.1.3DC-green)
![GLdc Version](https://img.shields.io/badge/GLdc-Latest-orange)
### Now Added: Primitive Types for OpenGL DC V.1.0

The following primitive types are taken from OpenGL for compatibility purposes. Please note that **not all types are fully implemented** in OpenGL DC Version 1.0.

```c
/* Primitive Types taken from GL for compatibility */
/* Not all types are implemented in OpenGL DC V.1.0 */
#define GL_POINTS                               0x0000
#define GL_LINES                                0x0001
#define GL_LINE_LOOP                            0x0002
#define GL_LINE_STRIP                           0x0003
#define GL_TRIANGLES                            0x0004
#define GL_TRIANGLE_STRIP                       0x0005
#define GL_TRIANGLE_FAN                         0x0006
#define GL_QUADS                                0x0007
#define GL_QUAD_STRIP                           0x0008
#define GL_POLYGON                              0x0009


### 📦 Precompiled Libraries and Headers
These libraries and headers are **precompiled** and fully working with the following setup:

```
OUTPUT:> KallistiOS v2.1.0
OUTPUT:> Git revision: v2.0.0-1542-gcd4e5db0  
         sh-elf-gcc (GCC) 13.2.0
```

⚠️ **Note:** These headers differ from those in `kos-ports`. To ensure correct compilation, **keep the headers local** and avoid looking in the main path, or you'll get incorrect headers and compile failures.

---

### 🖼️ SDL Framebuffer (Non-OpenGL)

For **2D framebuffer rendering** using SDL (without OpenGL), include the following headers:

```c
#include "include/SDL.h"
#include "include/SDL_dreamcast.h"
```

Your link line should look like this to include all current libraries:

```bash
-L$(KOS_BASE)/lib -lgcc -lSDL -lSDL_image -lpng -lz -ljpeg -lSDL -lSDL_mixer_126 -loggvorbisplay -ltremor -lm $(KOS_LIBS)
```

✅ **Important:** The order of libraries matters! If not linked in this order, you may encounter undefined references and compilation errors.

**📁 Place the `.lib` or `.a` files** in:
```
opt/toolchains/dc/kos/addons/lib/dreamcast
```

---

### 🎨 Hardware 3D Rendering (OpenGL)

For **3D hardware rendering** using SDL with OpenGL on Dreamcast, your link line should include:

```bash
$(OBJS) $(OBJEXTRA) -lSDL_gl -lGLdc -lm $(KOS_LIBS)
```

- **SDL framebuffer** is used for fast 2D blitting.
- **SDL-GLdc** is for hardware-accelerated 3d&2d OpenGL rendering with PVR.

---

### 📚 Setup Overview

- **KallistiOS:** v2.1.0
- **SDL Version:** 1.2.1.3DC (Framebuffer & OpenGL)
- **GCC Version:** 13.2.0


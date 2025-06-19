#include "scene.h"

static void scene_prepare_texture(Scene *scene, int id);
static void scene_set_texture_pixels(Scene *scene, int id);
static void scene_initialise_polygon_2d(Scene *scene, int polygon_index);

#ifdef RENDER_SW
static void scene_texture128_scanline(int32_t *restrict raster,
                                      int32_t *restrict texture, int k, int l,
                                      int i1, int j1, int k1, int l1,
                                      int length, int k2, int l2);
static void scene_texture128_alphakey_scanline(int32_t *restrict raster,
                                               int32_t *restrict texture, int l,
                                               int i1, int j1, int k1, int l1,
                                               int i2, int length, int l2,
                                               int i3);
static void scene_texture64_scanline(int32_t *restrict raster,
                                     int32_t *restrict texture, int k, int l,
                                     int i1, int j1, int k1, int l1, int length,
                                     int k2, int l2);
static void scene_texture64_alphakey_scanline(int32_t *restrict raster,
                                              int32_t *restrict texture, int l,
                                              int i1, int j1, int k1, int l1,
                                              int i2, int length, int l2,
                                              int i3);
static void scene_colour_translucent_scanline(int32_t *restrict raster, int i,
                                              int32_t *restrict ramp,
                                              int ramp_index, int ramp_inc);
static void scene_colour_scanline(int32_t *restrict raster, int i,
                                  int32_t *restrict ramp, int ramp_index,
                                  int ramp_inc);
static void scene_generate_scanlines(Scene *scene, int plane, int32_t *plane_x,
                                     int32_t *plane_y, int32_t *vertex_shade,
                                     GameModel *game_model, int face);
static void scene_rasterize(Scene *scene, int vertex_count, int32_t *vertices_x,
                            int32_t *vertices_y, int32_t *vertices_z,
                            int face_fill, GameModel *game_model);
static int scene_method306(int i, int j, int k, int l, int i1);
static int scene_method307(int i, int j, int k, int l, int flag);
static int scene_method308(int i, int j, int k, int flag);
static void scene_polygons_intersect_sort(Scene *scene, int step,
                                          GamePolygon **polygons, int count);
static int scene_polygons_order(Scene *scene, GamePolygon **polygons, int start,
                                int end);
static void scene_polygons_intersect_sort(Scene *scene, int step,
                                          GamePolygon **polygons, int count);
static int scene_polygons_order(Scene *scene, GamePolygon **polygons, int start,
                                int end);
static int scene_intersect(int *vertex_view_x_a, int *vertex_view_y_a,
                           int *vertex_view_x_b, int *vertex_view_y_b,
                           int length_a, int length_b);
static int scene_separate_polygon(GamePolygon *polygon_a,
                                  GamePolygon *polygon_b);
static int scene_heuristic_polygon(GamePolygon *polygon_a,
                                   GamePolygon *polygon_b);
static void scene_initialise_polygon_3d(Scene *scene, int polygon_index);
#endif

void scene_mouse_pick(Scene *scene, GameModel *model, int face) {
    if (scene->mouse_picked_count > (scene->max_mouse_picked / 2)) {
        size_t new_max = scene->max_mouse_picked * 2;
        void *new_ptr = NULL;

        new_ptr =
            realloc(scene->mouse_picked_models, new_max * sizeof(GameModel *));

        if (new_ptr == NULL) {
            return;
        }

        scene->mouse_picked_models = new_ptr;

        new_ptr = realloc(scene->mouse_picked_faces, new_max * sizeof(int));

        if (new_ptr == NULL) {
            return;
        }

        scene->mouse_picked_faces = new_ptr;
        scene->max_mouse_picked = new_max;
    }

    scene->mouse_picked_models[scene->mouse_picked_count] = model;
    scene->mouse_picked_faces[scene->mouse_picked_count] = face;
    scene->mouse_picked_count++;
}

int scene_frustum_max_x = 0;
int scene_frustum_min_x = 0;
int scene_frustum_max_y = 0;
int scene_frustum_min_y = 0;
int scene_frustum_far_z = 0;
int scene_frustum_near_z = 0;
int64_t scene_texture_count_loaded = 0;

int scene_polygon_depth_compare(const void *a, const void *b) {
    GamePolygon *polygon_a = (*(GamePolygon **)a);
    GamePolygon *polygon_b = (*(GamePolygon **)b);

    if (polygon_a->depth == 0) {
        return -1;
    }

    if (polygon_a->depth == polygon_b->depth) {
        return 0;
    }

    return polygon_a->depth < polygon_b->depth ? 1 : -1;
}

void scene_new(Scene *scene, Surface *surface, int model_count,
               int polygon_count, int max_sprite_count) {
    memset(scene, 0, sizeof(Scene));

    // TODO we need to re-allocate more polygons when client is resized, or just
    // add more to initial polygon_count
    polygon_count = 32767;

    scene->surface = surface;
    scene->max_model_count = model_count;
    scene->max_polygon_count = polygon_count;
    scene->max_sprite_count = max_sprite_count;

    scene->max_mouse_picked = 32;

    scene->mouse_picked_models =
        calloc(scene->max_mouse_picked, sizeof(GameModel *));

    scene->mouse_picked_faces = calloc(scene->max_mouse_picked, sizeof(int));

#ifndef RENDER_SW
    scene->gl_mouse_picked_size = 32;

    scene->gl_mouse_picked_time =
        calloc(scene->gl_mouse_picked_size, sizeof(GlModelTime *));
#endif

    scene->clip_near = 5;
    // scene->view_distance = 9;
    scene->view_distance = 512;
    scene->normal_magnitude = 4;

#ifdef RENDER_SW
    scene->raster = surface->pixels;
#endif

    scene->models = calloc(model_count, sizeof(GameModel *));
    scene->visible_polygons = calloc(polygon_count, sizeof(GamePolygon *));

    for (int i = 0; i < polygon_count; i++) {
        scene->visible_polygons[i] = malloc(sizeof(GamePolygon));
        polygon_new(scene->visible_polygons[i]);
    }

    GameModel *view = malloc(sizeof(GameModel));

    /* 2D sprites */
    game_model_new_alloc(view, scene->max_sprite_count * 2,
                         scene->max_sprite_count);

    scene->view = view;

    scene->sprite_id = calloc(max_sprite_count, sizeof(int));
    scene->sprite_width = calloc(max_sprite_count, sizeof(int));
    scene->sprite_height = calloc(max_sprite_count, sizeof(int));
    scene->sprite_x = calloc(max_sprite_count, sizeof(int));
    scene->sprite_z = calloc(max_sprite_count, sizeof(int));
    scene->sprite_y = calloc(max_sprite_count, sizeof(int));
    scene->sprite_translate_x = calloc(max_sprite_count, sizeof(int));

#if defined(RENDER_GL) || defined(RENDER_3DS_GL)
    scene->gl_sprite_depth_top = calloc(max_sprite_count, sizeof(float));
    scene->gl_sprite_depth_bottom = calloc(max_sprite_count, sizeof(float));

    scene->gl_terrain_buffer_length = 0;

    scene->gl_wall_buffers = calloc(1, sizeof(gl_vertex_buffer *));
    scene->gl_wall_buffers[0] = calloc(1, sizeof(gl_vertex_buffer));

    game_model_gl_create_buffer(scene->gl_wall_buffers[0], WALL_OBJECTS_MAX * 4,
                                WALL_OBJECTS_MAX * 6);
#endif

#ifdef RENDER_GL
#ifdef EMSCRIPTEN
    shader_new(&scene->game_model_shader, "./cache/game-model.webgl.vs",
               "./cache/game-model.webgl.fs");

    shader_new(&scene->game_model_pick_shader, "./cache/pick.webgl.vs",
               "./cache/pick.webgl.fs");
#elif defined(OPENGL15) || defined(OPENGL20)
    shader_new(&scene->game_model_shader, "./cache/game-model.gl2.vs",
               "./cache/game-model.gl2.fs");
#elif defined(__SWITCH__)
    shader_new(&scene->game_model_shader, "romfs:/game-model.vs",
               "romfs:/game-model.fs");
#else
    shader_new(&scene->game_model_shader, "./cache/game-model.vs",
               "./cache/game-model.fs");
#endif

    for (int i = 0; i < RAMP_SIZE; i++) {
        int gradient_index = (RAMP_SIZE - 1) - i;

        scene->light_gradient[gradient_index] =
            (i * i) / (float)(RAMP_SIZE * RAMP_SIZE);

        /*printf("%d %f\n", i,
               ((i / 4) * (i / 4)) / (float)(RAMP_SIZE * RAMP_SIZE));*/

        int texture_gradient_index = i / 16;
        int x = texture_gradient_index / 4;
        int y = texture_gradient_index % 4;

        /* thanks jorsa */
        scene->texture_light_gradient[gradient_index] =
            ((19 * pow(2, x)) + (4 * pow(2, x) * y)) / 255.0f;

        /*scene->texture_light_gradient[gradient_index] =
            0.074708f * powf(15.844317f, ((float)i / 255.0f));*/
    }

    shader_use(&scene->game_model_shader);

    shader_set_int(&scene->game_model_shader, "model_textures", 0);

    shader_set_float_array(&scene->game_model_shader, "light_gradient",
                           scene->light_gradient, RAMP_SIZE);

    shader_set_float_array(&scene->game_model_shader, "texture_light_gradient",
                           scene->texture_light_gradient, RAMP_SIZE);

#ifdef __SWITCH__
    gl_load_texture(&scene->gl_model_texture,
                    "romfs:/textures/model_textures.png");

    scene->gl_model_surface = IMG_Load("romfs:/textures/model_textures.png");
#else
    gl_load_texture(&scene->gl_model_texture,
                    "./cache/textures/model_textures.png");

#ifndef DREAMCAST
    scene->gl_model_surface = IMG_Load("./cache/textures/model_textures.png");
#endif
#ifdef DREAMCAST
    gl_load_texture(&scene->gl_model_texture, "./cache/textures/model_textures.png");
#endif
#endif
#elif defined(RENDER_3DS_GL)
    scene->_3ds_gl_model_shader_dvlb =
        DVLB_ParseFile((u32 *)model_shbin, model_shbin_size);

    shaderProgramInit(&scene->_3ds_gl_model_shader);

    shaderProgramSetVsh(&scene->_3ds_gl_model_shader,
                        &scene->_3ds_gl_model_shader_dvlb->DVLE[0]);

    C3D_BindProgram(&scene->_3ds_gl_model_shader);

    scene->_3ds_gl_model_uniform = shaderInstanceGetUniformLocation(
        (&scene->_3ds_gl_model_shader)->vertexShader, "scroll_texture");

    scene->_3ds_gl_model_uniform = shaderInstanceGetUniformLocation(
        (&scene->_3ds_gl_model_shader)->vertexShader, "model");

    scene->_3ds_gl_light_ambience_diffuse_fog_uniform =
        shaderInstanceGetUniformLocation(
            (&scene->_3ds_gl_model_shader)->vertexShader,
            "light_ambience_diffuse_fog");

    scene->_3ds_gl_unlit_uniform = shaderInstanceGetUniformLocation(
        (&scene->_3ds_gl_model_shader)->vertexShader, "unlit");

    scene->_3ds_gl_light_direction_uniform = shaderInstanceGetUniformLocation(
        (&scene->_3ds_gl_model_shader)->vertexShader, "light_direction");

    scene->_3ds_gl_opacity_uniform = shaderInstanceGetUniformLocation(
        (&scene->_3ds_gl_model_shader)->vertexShader, "opacity");

    scene->_3ds_gl_projection_view_model_uniform =
        shaderInstanceGetUniformLocation(
            (&scene->_3ds_gl_model_shader)->vertexShader,
            "projection_view_model");

    scene->_3ds_gl_cull_front_uniform = shaderInstanceGetUniformLocation(
        (&scene->_3ds_gl_model_shader)->vertexShader, "cull_front");

    _3ds_gl_load_tex(model_textures_t3x, model_textures_t3x_size,
                     &scene->gl_model_texture);
#endif
}

#ifdef RENDER_SW
static void scene_texture128_scanline(int32_t *restrict raster,
                                      int32_t *restrict texture, int k, int l,
                                      int i1, int j1, int k1, int l1,
                                      int length, int k2, int l2) {
    // 2 ** 7 = 128
    static const int texture_shift = 7;
    const int texture_size = (int)pow(2, texture_shift);
    const int texture_area = (texture_size * texture_size) - texture_size;

    if (length <= 0) {
        return;
    }

    int i = 0;
    int j = 0;
    int i3 = 0;
    int j3 = 0;
    int i4 = 0;

    if (i1 != 0) {
        i = (k / i1) << texture_shift;
        j = (l / i1) << texture_shift;
    }

    if (i < 0) {
        i = 0;
    } else if (i > texture_area) {
        i = texture_area;
    }

    k += j1;
    l += k1;
    i1 += l1;

    if (i1 != 0) {
        i3 = (k / i1) << texture_shift;
        j3 = (l / i1) << texture_shift;
    }

    if (i3 < 0) {
        i3 = 0;
    } else if (i3 > texture_area) {
        i3 = texture_area;
    }

    int k3 = (i3 - i) / 16;
    int l3 = (j3 - j) / 16;

    for (int i_ = length / 16; i_ > 0; i_--) {
        for (int j_ = 0; j_ < 4; j_++) {
            i = (i & (texture_area + (texture_size - 1))) + (k2 & 0x600000);
            i4 = k2 >> 23;
            k2 += l2;

            for (int k_ = 0; k_ < 4; k_++) {
                (*raster++) =
                    texture[(j & texture_area) + (i >> texture_shift)] >> i4;

                i += k3;
                j += l3;
            }
        }

        i = i3;
        j = j3;

        k += j1;
        l += k1;
        i1 += l1;

        if (i1 != 0) {
            i3 = (k / i1) << texture_shift;
            j3 = (l / i1) << texture_shift;
        }

        if (i3 < 0) {
            i3 = 0;
        } else if (i3 > texture_area) {
            i3 = texture_area;
        }

        k3 = (i3 - i) / 16;
        l3 = (j3 - j) / 16;
    }

    for (int i_ = 0; i_ < (length & 0xf); i_++) {
        if ((i_ & 3) == 0) {
            i = (i & (texture_area + (texture_size - 1))) + (k2 & 0x600000);
            i4 = k2 >> 23;
            k2 += l2;
        }

        (*raster++) = texture[(j & texture_area) + (i >> texture_shift)] >> i4;

        i += k3;
        j += l3;
    }
}

static void scene_texture128_alphakey_scanline(int32_t *restrict raster,
                                               int32_t *restrict texture, int l,
                                               int i1, int j1, int k1, int l1,
                                               int i2, int length, int l2,
                                               int i3) {
    // 2 ** 7 = 128
    static const int texture_shift = 7;
    const int texture_size = (int)pow(2, texture_shift);
    const int texture_area = (texture_size * texture_size) - texture_size;

    if (length <= 0) {
        return;
    }

    int colour = 0;
    int j = 0;
    int k = 0;
    int j3 = 0;
    int k3 = 0;
    i3 <<= 2;

    if (j1 != 0) {
        j3 = (l / j1) << texture_shift;
        k3 = (i1 / j1) << texture_shift;
    }

    if (j3 < 0) {
        j3 = 0;
    } else if (j3 > texture_area) {
        j3 = texture_area;
    }

    for (int i_ = length; i_ > 0; i_ -= 16) {
        l += k1;
        i1 += l1;
        j1 += i2;
        j = j3;
        k = k3;

        if (j1 != 0) {
            j3 = (l / j1) << texture_shift;
            k3 = (i1 / j1) << texture_shift;
        }

        if (j3 < 0) {
            j3 = 0;
        } else if (j3 > texture_area) {
            j3 = texture_area;
        }

        int l3 = (j3 - j) >> 4;
        int i4 = (k3 - k) >> 4;
        int k4 = l2 >> 23;

        j += l2 & 0x600000;
        l2 += i3;

        if (i_ < 16) {
            for (int j_ = 0; j_ < i_; j_++) {
                if ((colour =
                         texture[(k & texture_area) + (j >> texture_shift)] >>
                         k4) != 0) {
                    (*raster) = colour;
                }

                raster++;
                j += l3;
                k += i4;

                if ((j_ & 3) == 3) {
                    j = (j & (texture_area + texture_size - 1)) +
                        (l2 & 0x600000);

                    k4 = l2 >> 23;
                    l2 += i3;
                }
            }
        } else {
            for (int j_ = 0; j_ < 4; j_++) {
                for (int k_ = 0; k_ < 4; k_++) {
                    if ((colour = texture[(k & texture_area) +
                                          (j >> texture_shift)] >>
                                  k4) != 0) {
                        (*raster) = colour;
                    }

                    raster++;
                    j += l3;
                    k += i4;
                }

                if (j_ == 3) {
                    break;
                }

                j = (j & (texture_area + texture_size - 1)) + (l2 & 0x600000);

                k4 = l2 >> 23;
                l2 += i3;
            }
        }
    }
}

static void scene_texture64_scanline(int32_t *restrict raster,
                                     int32_t *restrict texture, int k, int l,
                                     int i1, int j1, int k1, int l1, int length,
                                     int k2, int l2) {
    // 2 ** 6 = 64
    static const int texture_shift = 6;
    int texture_size = (int)pow(2, texture_shift);
    const int texture_area = (texture_size * texture_size) - texture_size;

    if (length <= 0) {
        return;
    }

    int i = 0;
    int j = 0;
    int i3 = 0;
    int j3 = 0;
    l2 <<= 2; // * 4

    if (i1 != 0) {
        i3 = (k / i1) << texture_shift;
        j3 = (l / i1) << texture_shift;
    }

    if (i3 < 0) {
        i3 = 0;
    } else if (i3 > texture_area) {
        i3 = texture_area;
    }

    for (int i_ = length; i_ > 0; i_ -= 16) {
        k += j1;
        l += k1;
        i1 += l1;
        i = i3;
        j = j3;

        if (i1 != 0) {
            i3 = (k / i1) << texture_shift;
            j3 = (l / i1) << texture_shift;
        }

        if (i3 < 0) {
            i3 = 0;
        } else if (i3 > texture_area) {
            i3 = texture_area;
        }

        int k3 = (i3 - i) >> 4;
        int l3 = (j3 - j) >> 4;
        int32_t j4 = k2 >> 20;

        i += k2 & 0xc0000;
        k2 += l2;

        if (i_ < 16) {
            for (int j_ = 0; j_ < i_; j_++) {
                (*raster++) =
                    texture[(j & texture_area) + (i >> texture_shift)] >> j4;

                i += k3;
                j += l3;

                if ((j_ & 3) == 3) {
                    i = (i & (texture_area + texture_size - 1)) +
                        (k2 & 0xc0000);

                    j4 = k2 >> 20;
                    k2 += l2;
                }
            }
        } else {
            for (int j_ = 0; j_ < 4; j_++) {
                for (int k_ = 0; k_ < 4; k_++) {
                    (*raster++) =
                        texture[(j & texture_area) + (i >> texture_shift)] >>
                        j4;

                    i += k3;
                    j += l3;
                }

                if (j_ == 3) {
                    break;
                }

                i = (i & (texture_area + texture_size - 1)) + (k2 & 0xc0000);
                j4 = k2 >> 20;
                k2 += l2;
            }
        }
    }
}

static void scene_texture64_alphakey_scanline(int32_t *restrict raster,
                                              int32_t *restrict texture, int l,
                                              int i1, int j1, int k1, int l1,
                                              int i2, int length, int l2,
                                              int i3) {
    // 2 ** 6 = 64
    static const int texture_shift = 6;
    const int texture_size = (int)pow(2, texture_shift);
    const int texture_area = (texture_size * texture_size) - texture_size;

    if (length <= 0) {
        return;
    }

    int colour = 0;
    int j = 0;
    int k = 0;
    int j3 = 0;
    int k3 = 0;
    i3 <<= 2;

    if (j1 != 0) {
        j3 = (l / j1) << texture_shift;
        k3 = (i1 / j1) << texture_shift;
    }

    if (j3 < 0) {
        j3 = 0;
    } else if (j3 > texture_area) {
        j3 = texture_area;
    }

    for (int i_ = length; i_ > 0; i_ -= 16) {
        l += k1;
        i1 += l1;
        j1 += i2;
        j = j3;
        k = k3;

        if (j1 != 0) {
            j3 = (l / j1) << texture_shift;
            k3 = (i1 / j1) << texture_shift;
        }

        if (j3 < 0) {
            j3 = 0;
        } else if (j3 > texture_area) {
            j3 = texture_area;
        }

        int l3 = (j3 - j) >> 4;
        int i4 = (k3 - k) >> 4;
        int k4 = l2 >> 20;

        j += l2 & 0xc0000;
        l2 += i3;

        if (i_ < 16) {
            for (int j_ = 0; j_ < i_; j_++) {
                if ((colour =
                         texture[(k & texture_area) + (j >> texture_shift)] >>
                         k4) != 0) {
                    (*raster) = colour;
                }

                raster++;
                j += l3;
                k += i4;

                if ((j_ & 3) == 3) {
                    j = (j & (texture_area + texture_size - 1)) +
                        (l2 & 0xc0000);

                    k4 = l2 >> 20;
                    l2 += i3;
                }
            }
        } else {
            for (int j_ = 0; j_ < 4; j_++) {
                for (int k_ = 0; k_ < 4; k_++) {
                    if ((colour = texture[(k & texture_area) +
                                          (j >> texture_shift)] >>
                                  k4) != 0) {
                        (*raster) = colour;
                    }

                    raster++;
                    j += l3;
                    k += i4;
                }

                if (j_ == 3) {
                    break;
                }

                j = (j & (texture_area + texture_size - 1)) + (l2 & 0xc0000);
                k4 = l2 >> 20;
                l2 += i3;
            }
        }
    }
}

static void scene_colour_translucent_scanline(int32_t *restrict raster, int i,
                                              int32_t *restrict ramp,
                                              int ramp_index, int ramp_inc) {
    /* If i >= 0, nothing to render */
    if (i >= 0) {
        return;
    }

    /* Multiply ramp increment by 4 as before */
    ramp_inc *= 4;

    /*
     * Grab initial color from ramp. Main loop handles sets of 16 pixels
     * in groups of 4, updating 'colour' (ew br*tish) after each 4-pixel block.
     */
    int colour = ramp[(ramp_index / RAMP_SIZE) & 0xff];
    ramp_index += ramp_inc;

    /* Each group of 16 pixels in the original code used 4x(4 pixel writes). */
    int length = i / 16;
    
    for (int block_i = length; block_i < 0; block_i++) {

#define WRITE_4_PIXELS() do {                                        \
    *raster = colour + ((*(raster + 1) >> 1) & 0x7f7f7f); raster++;   \
    *raster = colour + ((*(raster + 1) >> 1) & 0x7f7f7f); raster++;   \
    *raster = colour + ((*(raster + 1) >> 1) & 0x7f7f7f); raster++;   \
    *raster = colour + ((*(raster + 1) >> 1) & 0x7f7f7f); raster++;   \
    colour = ramp[(ramp_index / RAMP_SIZE) & 0xff];                  \
    ramp_index += ramp_inc;                                          \
} while(0)

        WRITE_4_PIXELS();  /* j=0 */
        WRITE_4_PIXELS();  /* j=1 */
        WRITE_4_PIXELS();  /* j=2 */
        WRITE_4_PIXELS();  /* j=3 */
#undef WRITE_4_PIXELS
    }

    /*
     * Remainder pass (i % 16 leftover pixels). length = -(i % 16).
     * The code does small strips, updating color after each group of 4 pixels.
     */
    length = -(i % 16); /* how many leftover pixels (0..15) */

    for (int pix_i = 0; pix_i < length; pix_i++) {
        *raster = colour + ((*(raster + 1) >> 1) & 0x7f7f7f);
        raster++;

        if ((pix_i & 3) == 3) {
            colour = ramp[(ramp_index / RAMP_SIZE) & 0xff];
            ramp_index += ramp_inc * 2;
        }
    }
}


static void scene_colour_scanline(int32_t *restrict raster, int i,
                                  int32_t *restrict ramp, int ramp_index,
                                  int ramp_inc) {
    if (i >= 0) {
        return;
    }

    /*
     * RAMP_WIDE decides if we step in blocks of 2 or 4 horizontally.
     * Default inner loop is either 8 or 16px.
     */
    int step = (RAMP_WIDE ? 2 : 4);
    ramp_inc *= (RAMP_WIDE ? 2 : 4);

    int colour = ramp[(ramp_index / RAMP_SIZE) & 0xff];
    ramp_index += ramp_inc;

    int block_size = (RAMP_WIDE ? 8 : 16);   /* how many pixels per block */
    int length = i / block_size;

    /* MAIN BLOCK LOOP */
    for (int block_i = length; block_i < 0; block_i++) {
#define WRITE_BLOCK() do {                              \
    for (int k_ = 0; k_ < step; k_++) {                 \
        *raster++ = colour;                             \
    }                                                   \
    colour = ramp[(ramp_index / RAMP_SIZE) & 0xff];     \
    ramp_index += ramp_inc;                             \
} while(0)

        WRITE_BLOCK();  /* j=0 */
        WRITE_BLOCK();  /* j=1 */
        WRITE_BLOCK();  /* j=2 */
        WRITE_BLOCK();  /* j=3 */
#undef WRITE_BLOCK
    }

    /*
     * Remainder pass: i % block_size leftover pixels
     * length = -(i % block_size)
     */
    length = -(i % block_size);
    int ramp_flag = (RAMP_WIDE ? 1 : 3);

    for (int pix_i = 0; pix_i < length; pix_i++) {
        *raster++ = colour;
        if ((pix_i & ramp_flag) == ramp_flag) {
            if (RAMP_WIDE) {
                colour = ramp[(ramp_index >> 8) & 0xff];
            } else {
                colour = ramp[(ramp_index / RAMP_SIZE) & 0xff];
            }
            ramp_index += ramp_inc;
        }
    }
}

#endif /* RENDER_SW */

void scene_add_model(Scene *scene, GameModel *model) {
    if (model == NULL) {
        mud_error("Warning tried to add null object!\n");
        return;
    }

    if (scene->model_count < scene->max_model_count) {
        scene->models[scene->model_count++] = model;
    }
}

void scene_remove_model(Scene *scene, GameModel *model) {
    for (int i = 0; i < scene->model_count; i++) {
        if (scene->models[i] == model) {
            scene->models[i] = NULL;
            scene->model_count--;

            for (int j = i; j < scene->model_count; j++) {
                scene->models[j] = scene->models[j + 1];
            }
        }
    }
}

void scene_null_model(Scene *scene, GameModel *model) {
    for (int i = 0; i < scene->model_count; i++) {
        if (scene->models[i] == model) {
            scene->models[i] = NULL;
        }
    }
}

void scene_dispose(Scene *scene) {
    for (int i = 0; i < scene->model_count; i++) {
        /* these need to be free'd elsewhere */
        scene->models[i] = NULL;
    }

    scene_clear(scene);

    scene->model_count = 0;
}

void scene_clear(Scene *scene) {
    scene->sprite_count = 0;

    for (int i = 0; i < scene->view->face_count; i++) {
        free(scene->view->face_vertices[i]);
    }

    game_model_clear(scene->view);
}

void scene_reduce_sprites(Scene *scene, int i) {
    scene->sprite_count -= i;

    game_model_reduce(scene->view, i, i * 2);

    if (scene->sprite_count < 0) {
        scene->sprite_count = 0;
    }
}

int scene_add_sprite(Scene *scene, int sprite_id, int x, int y, int z,
                     int width, int height, int tag) {
    scene->sprite_id[scene->sprite_count] = sprite_id;
    scene->sprite_x[scene->sprite_count] = x;
    scene->sprite_z[scene->sprite_count] = y;
    scene->sprite_y[scene->sprite_count] = z;
    scene->sprite_width[scene->sprite_count] = width;
    scene->sprite_height[scene->sprite_count] = height;
    scene->sprite_translate_x[scene->sprite_count] = 0;

#if defined(RENDER_GL) || defined(RENDER_3DS_GL)
    vec4 projected_position = {0};

    vec4 bottom_position = {VERTEX_TO_FLOAT(x), VERTEX_TO_FLOAT(y),
                            VERTEX_TO_FLOAT(z), 1.0f};

    glm_mat4_mulv(scene->gl_projection_view, bottom_position,
                  projected_position);

    scene->gl_sprite_depth_bottom[scene->sprite_count] =
        projected_position[2] / projected_position[3];

    glm_vec4_zero(projected_position);

    vec4 top_position = {VERTEX_TO_FLOAT(x),
                         VERTEX_TO_FLOAT(y) - VERTEX_TO_FLOAT(height),
                         VERTEX_TO_FLOAT(z), 1.0f};

    glm_mat4_mulv(scene->gl_projection_view, top_position, projected_position);

    scene->gl_sprite_depth_top[scene->sprite_count] =
        projected_position[2] / projected_position[3];

#endif

    // #ifdef RENDER_SW
    uint16_t *vertices = calloc(2, sizeof(int));

    vertices[0] = game_model_create_vertex(scene->view, x, y, z);
    vertices[1] = game_model_create_vertex(scene->view, x, y - height, z);

    game_model_create_face(scene->view, 2, vertices, 0, 0);

    scene->view->face_tag[scene->sprite_count] = tag;
    scene->view->is_local_player[scene->sprite_count++] = 0;
    // #endif

    return scene->sprite_count - 1;
}

void scene_set_local_player(Scene *scene, int i) {
    scene->view->is_local_player[i] = 1;
}

void scene_set_sprite_translate_x(Scene *scene, int i, int n) {
    scene->sprite_translate_x[i] = n;
}

void scene_set_mouse_location(Scene *scene, int x, int y) {
    scene->mouse_x = x - scene->base_x;
    scene->mouse_y = y;
    scene->mouse_picked_count = 0;
    scene->mouse_picking_active = 1;

#if defined(RENDER_GL) || defined(RENDER_3DS_GL)
    scene->gl_mouse_picked_count = 0;

    // TODO use inverse_projection_view
    float gl_x = gl_translate_coord(x, scene->width);
    float gl_y = -gl_translate_coord(y, scene->gl_height);

    vec4 clip = {gl_x, gl_y, -1.0f, 1.0f};

    vec4 eye = {0};
    glm_mat4_mulv(scene->gl_inverse_projection, clip, eye);

    eye[2] = -1.0f;
    eye[3] = 0;

    vec4 world = {0};
    glm_mat4_mulv(scene->gl_inverse_view, eye, world);

    scene->gl_mouse_ray[0] = world[0];
    scene->gl_mouse_ray[1] = world[1];
    scene->gl_mouse_ray[2] = world[2];

    glm_vec3_normalize(scene->gl_mouse_ray);
#endif
}

void scene_set_bounds(Scene *scene, int width, int height) {
    int base_x = width / 2;
    int base_y = height / 2;
    int clip_x = base_x;
    int clip_y = base_y;

    scene->clip_x = clip_x;
    scene->clip_y = clip_y;
    scene->base_x = base_x;
    scene->base_y = base_y;
    scene->width = width;

#ifdef RENDER_SW
    int scanlines_length = clip_y + base_y;

    scene->scanlines = calloc(scanlines_length, sizeof(Scanline));
#endif

#if defined(RENDER_GL) || defined(RENDER_3DS_GL)
    scene->gl_height = height;
#endif
}

#ifdef RENDER_SW
static void scene_polygons_intersect_sort(Scene *scene, int step,
                                          GamePolygon **polygons, int count) {
    for (int i = 0; i <= count; i++) {
        polygons[i]->visited = 0;
        polygons[i]->index = i;
        polygons[i]->index2 = -1;
    }

    int l = 0;

    do {
        while (polygons[l]->visited) {
            l++;
        }

        if (l == count) {
            return;
        }

        GamePolygon *polygon = polygons[l];
        polygon->visited = 1;

        int start = l;
        int end = l + step;

        if (end >= count) {
            end = count - 1;
        }

        for (int i = end; i >= start + 1; i--) {
            GamePolygon *other = polygons[i];

            if (polygon->min_plane_x < other->max_plane_x &&
                other->min_plane_x < polygon->max_plane_x &&
                polygon->min_plane_y < other->max_plane_y &&
                other->min_plane_y < polygon->max_plane_y &&
                polygon->index != other->index2 &&
                !scene_separate_polygon(polygon, other) &&
                scene_heuristic_polygon(other, polygon)) {
                scene_polygons_order(scene, polygons, start, i);

                if (polygons[i] != other) {
                    i++;
                }

                start = scene->new_start;
                other->index2 = polygon->index;
            }
        }
    } while (1);
}

static int scene_polygons_order(Scene *scene, GamePolygon **polygons, int start,
                                int end) {
    do {
        GamePolygon *polygon_start = polygons[start];

        for (int i = start + 1; i <= end; i++) {
            GamePolygon *polygon = polygons[i];

            if (!scene_separate_polygon(polygon, polygon_start)) {
                break;
            }

            polygons[start] = polygon;
            polygons[i] = polygon_start;
            start = i;

            if (start == end) {
                scene->new_start = start;
                scene->new_end = start - 1;
                return 1;
            }
        }

        GamePolygon *polygon_end = polygons[end];

        for (int i = end - 1; i >= start; i--) {
            GamePolygon *polygon = polygons[i];

            if (!scene_separate_polygon(polygon_end, polygon)) {
                break;
            }

            polygons[end] = polygon;
            polygons[i] = polygon_end;
            end = i;

            if (start == end) {
                scene->new_start = end + 1;
                scene->new_end = end;
                return 1;
            }
        }

        if (start + 1 >= end) {
            scene->new_start = start;
            scene->new_end = end;
            return 0;
        }

        if (!scene_polygons_order(scene, polygons, start + 1, end)) {
            scene->new_start = start;
            return 0;
        }

        end = scene->new_end;
    } while (1);
}
#endif /* RENDER_SW */

void scene_set_frustum(Scene *scene, int x, int y, int z) {
    int yaw = (-scene->camera_yaw + 1024) & 1023;
    int pitch = (-scene->camera_pitch + 1024) & 1023;
    int roll = (-scene->camera_roll + 1024) & 1023;

    if (roll != 0) {
        int sine = sin_cos_2048[roll];
        int cosine = sin_cos_2048[roll + 1024];
        int i3 = (y * sine + x * cosine) / 32768;
        y = (y * cosine - x * sine) / 32768;
        x = i3;
    }

    if (yaw != 0) {
        int sine = sin_cos_2048[yaw];
        int cosine = sin_cos_2048[yaw + 1024];
        int j3 = (y * cosine - z * sine) / 32768;
        z = (y * sine + z * cosine) / 32768;
        y = j3;
    }

    if (pitch != 0) {
        int sine = sin_cos_2048[pitch];
        int cosine = sin_cos_2048[pitch + 1024];
        int k3 = (z * sine + x * cosine) / 32768;
        z = (z * cosine - x * sine) / 32768;
        x = k3;
    }

    if (x < scene_frustum_max_x) {
        scene_frustum_max_x = x;
    }

    if (x > scene_frustum_min_x) {
        scene_frustum_min_x = x;
    }

    if (y < scene_frustum_max_y) {
        scene_frustum_max_y = y;
    }

    if (y > scene_frustum_min_y) {
        scene_frustum_min_y = y;
    }

    if (z < scene_frustum_far_z) {
        scene_frustum_far_z = z;
    }

    if (z > scene_frustum_near_z) {
        scene_frustum_near_z = z;
    }
}

static void scene_initialise_polygon_2d(Scene *scene, int polygon_index) {
    GamePolygon *polygon = scene->visible_polygons[polygon_index];
    GameModel *game_model = polygon->model;
    int face = polygon->face;
    uint16_t *face_vertices = game_model->face_vertices[face];

    game_model->normal_magnitude[face] = 1;
    game_model->normal_scale[face] = 0;

    polygon->visibility = game_model->project_vertex_z[face_vertices[0]];
    polygon->normal_x = 0;
    polygon->normal_y = 0;
    polygon->normal_z = 1;

    int min_z = game_model->project_vertex_z[face_vertices[0]];
    int max_z = min_z;
    int min_plane_x = game_model->vertex_view_x[face_vertices[0]];
    int max_plane_x = min_plane_x;

    if (game_model->vertex_view_x[face_vertices[1]] < min_plane_x) {
        min_plane_x = game_model->vertex_view_x[face_vertices[1]];
    } else {
        max_plane_x = game_model->vertex_view_x[face_vertices[1]];
    }

    int min_plane_y = game_model->vertex_view_y[face_vertices[1]];
    int max_plane_y = game_model->vertex_view_y[face_vertices[0]];
    int project_vertex = game_model->project_vertex_z[face_vertices[1]];

    if (project_vertex > max_z) {
        max_z = project_vertex;
    } else if (project_vertex < min_z) {
        min_z = project_vertex;
    }

    project_vertex = game_model->vertex_view_x[face_vertices[1]];

    if (project_vertex > max_plane_x) {
        max_plane_x = project_vertex;
    } else if (project_vertex < min_plane_x) {
        min_plane_x = project_vertex;
    }

    project_vertex = game_model->vertex_view_y[face_vertices[1]];

    if (project_vertex > max_plane_y) {
        max_plane_y = project_vertex;
    } else if (project_vertex < min_plane_y) {
        min_plane_y = project_vertex;
    }

    polygon->min_z = min_z;
    polygon->max_z = max_z;
    polygon->min_plane_x = min_plane_x - 20;
    polygon->max_plane_x = max_plane_x + 20;
    polygon->min_plane_y = min_plane_y;
    polygon->max_plane_y = max_plane_y;
}

void scene_initialise_polygons_2d(Scene *scene) {
    for (int face = 0; face < scene->view->face_count; face++) {
        uint16_t *face_vertices = scene->view->face_vertices[face];
        uint16_t face_vertex_index = face_vertices[0];
        int project_z = scene->view->project_vertex_z[face_vertex_index];

        if (project_z < scene->clip_near || project_z > scene->clip_far_2d) {
            continue;
        }

        int view_x = scene->view->vertex_view_x[face_vertex_index];
        int view_y = scene->view->vertex_view_y[face_vertex_index];

        int view_width =
            //(scene->sprite_width[face] << scene->view_distance) / view_z;
            (scene->sprite_width[face] * scene->view_distance) / project_z;

        int view_height =
            //(scene->sprite_height[face] << scene->view_distance) / view_z;
            (scene->sprite_height[face] * scene->view_distance) / project_z;

        if (view_x - (view_width / 2) <= scene->clip_x &&
            view_x + (view_width / 2) >= -scene->clip_x &&
            view_y - view_height <= scene->clip_y && view_y >= -scene->clip_y) {

            if (scene->visible_polygons_count >=
                (scene->max_polygon_count - 1)) {
                break;
            }

            GamePolygon *polygon =
                scene->visible_polygons[scene->visible_polygons_count];

            polygon->model = scene->view;
            polygon->face = face;

            scene_initialise_polygon_2d(scene, scene->visible_polygons_count);

            polygon->depth =
                (project_z + scene->view->project_vertex_z[face_vertices[1]]) /
                2;

            scene->visible_polygons_count++;
        }
    }
}

void scene_render_polygon_2d_face(Scene *scene, int face) {
    uint16_t *face_vertices = scene->view->face_vertices[face];
    uint16_t face_0 = face_vertices[0];
    int view_x = scene->view->vertex_view_x[face_0];
    int view_y = scene->view->vertex_view_y[face_0];
    int project_z = scene->view->project_vertex_z[face_0];

    // int width = (scene->sprite_width[face] << scene->view_distance) / vz;
    int width = (scene->sprite_width[face] * scene->view_distance) / project_z;
    // int height = (scene->sprite_height[face] << scene->view_distance) / vz;

    int height =
        (scene->sprite_height[face] * scene->view_distance) / project_z;

    int skew_x = scene->view->vertex_view_x[face_vertices[1]] - view_x;
    int x = view_x - (width / 2);
    int y = scene->base_y + view_y - height;

    float depth_top = 0;
    float depth_bottom = 0;

#if defined(RENDER_GL) || defined(RENDER_3DS_GL)
    depth_top = scene->gl_sprite_depth_top[face];
    depth_bottom = scene->gl_sprite_depth_bottom[face];
#endif

    /*surface_draw_entity_sprite(
        scene->surface, x + scene->base_x, y, width, height,
       scene->sprite_id[face], skew_x, (256 << scene->view_distance) /
       project_z, depth_top, depth_bottom);*/

    // mud_log("software position: %d %d\n", x, view_x);

    surface_draw_entity_sprite(scene->surface, x + scene->base_x, y, width,
                               height, scene->sprite_id[face], skew_x,
                               (256 * scene->view_distance) / project_z,
                               depth_top, depth_bottom);

    if (scene->mouse_picking_active) {
        // x += (scene->sprite_translate_x[face] << scene->view_distance) /
        // project_z;
        x += (scene->sprite_translate_x[face] * scene->view_distance) /
             project_z;

        if (scene->mouse_y >= y && scene->mouse_y <= y + height &&
            scene->mouse_x >= x && scene->mouse_x <= x + width &&
            !scene->view->unpickable &&
            scene->view->is_local_player[face] == 0) {

            scene_mouse_pick(scene, scene->view, face);
        }
    }
}

void scene_render(Scene *scene) {
    scene->interlace = scene->surface->interlace;

    int frustum_x =
        //(scene->clip_x * scene->clip_far_3d) >> scene->view_distance;
        (scene->clip_x * scene->clip_far_3d) / scene->view_distance;

    int frustum_y =
        //(scene->clip_y * scene->clip_far_3d) >> scene->view_distance;
        (scene->clip_y * scene->clip_far_3d) / scene->view_distance;

    scene_frustum_max_x = 0;  /* right */
    scene_frustum_min_x = 0;  /* left */
    scene_frustum_max_y = 0;  /* top */
    scene_frustum_min_y = 0;  /* bottom */
    scene_frustum_far_z = 0;  /* far */
    scene_frustum_near_z = 0; /* near */

    scene_set_frustum(scene, -frustum_x, -frustum_y, scene->clip_far_3d);
    scene_set_frustum(scene, -frustum_x, frustum_y, scene->clip_far_3d);
    scene_set_frustum(scene, frustum_x, -frustum_y, scene->clip_far_3d);
    scene_set_frustum(scene, frustum_x, frustum_y, scene->clip_far_3d);
    scene_set_frustum(scene, -scene->clip_x, -scene->clip_y, 0);
    scene_set_frustum(scene, -scene->clip_x, scene->clip_y, 0);
    scene_set_frustum(scene, scene->clip_x, -scene->clip_y, 0);
    scene_set_frustum(scene, scene->clip_x, scene->clip_y, 0);

    scene_frustum_max_x += scene->camera_x;
    scene_frustum_min_x += scene->camera_x;
    scene_frustum_max_y += scene->camera_y;
    scene_frustum_min_y += scene->camera_y;
    scene_frustum_far_z += scene->camera_z;
    scene_frustum_near_z += scene->camera_z;

    scene->models[scene->model_count] = scene->view;
    scene->view->transform_state = GAME_MODEL_TRANSFORM_RESET;

    for (int i = 0; i <= scene->model_count; i++) {
        game_model_project(scene->models[i], scene->camera_x, scene->camera_y,
                           scene->camera_z, scene->camera_yaw,
                           scene->camera_pitch, scene->camera_roll,
                           scene->view_distance, scene->clip_near);
    }

#ifdef RENDER_SW
    scene->visible_polygons_count = 0;

    for (int i = 0; i < scene->model_count; i++) {
        GameModel *game_model = scene->models[i];

        if (!game_model->visible) {
            continue;
        }

        for (int face = 0; face < game_model->face_count; face++) {
            uint16_t vertex_count = game_model->face_vertex_count[face];
            uint16_t *face_vertices = game_model->face_vertices[face];
            int visible = 0;

            for (int vertex = 0; vertex < vertex_count; vertex++) {
                int z = game_model->project_vertex_z[face_vertices[vertex]];

                if (z <= scene->clip_near || z >= scene->clip_far_3d) {
                    continue;
                }

                visible = 1;
                break;
            }

            if (visible) {
                int view_x_count = 0;

                for (int vertex = 0; vertex < vertex_count; vertex++) {
                    int x = game_model->vertex_view_x[face_vertices[vertex]];

                    if (x > -scene->clip_x) {
                        view_x_count |= 1;
                    }

                    if (x < scene->clip_x) {
                        view_x_count |= 2;
                    }

                    if (view_x_count == 3) {
                        break;
                    }
                }

                if (view_x_count == 3) {
                    int view_y_count = 0;

                    for (int vertex = 0; vertex < vertex_count; vertex++) {
                        int vertex_y =
                            game_model->vertex_view_y[face_vertices[vertex]];

                        if (vertex_y > -scene->clip_y) {
                            view_y_count |= 1;
                        }

                        if (vertex_y < scene->clip_y) {
                            view_y_count |= 2;
                        }

                        if (view_y_count == 3) {
                            break;
                        }
                    }

                    if (view_y_count == 3) {
                        if (scene->visible_polygons_count >=
                            (scene->max_polygon_count - 1)) {
                            break;
                        }

                        GamePolygon *polygon_1 =
                            scene->visible_polygons
                                [scene->visible_polygons_count];

                        polygon_1->model = game_model;
                        polygon_1->face = face;

                        scene_initialise_polygon_3d(
                            scene, scene->visible_polygons_count);

                        int face_fill = 0;

                        if (polygon_1->visibility < 0) {
                            face_fill = game_model->face_fill_front[face];
                        } else {
                            face_fill = game_model->face_fill_back[face];
                        }

                        if (face_fill != COLOUR_TRANSPARENT) {
                            int h = 0;

                            for (int vertex = 0; vertex < vertex_count;
                                 vertex++) {
                                h += game_model->project_vertex_z
                                         [face_vertices[vertex]];
                            }

                            polygon_1->depth =
                                (h / vertex_count) + game_model->depth;

#ifdef USE_TOONSCAPE
                            face_fill = apply_toonscape(face_fill);
#endif
                            polygon_1->facefill = face_fill;

                            scene->visible_polygons_count++;
                        }
                    }
                }
            }
        }
    }

    scene_initialise_polygons_2d(scene);

    if (scene->visible_polygons_count == 0) {
        return;
    }

    scene->last_visible_polygons_count = scene->visible_polygons_count;

    qsort(scene->visible_polygons, scene->visible_polygons_count,
          sizeof(GamePolygon *), scene_polygon_depth_compare);

    scene_polygons_intersect_sort(scene, 100, scene->visible_polygons,
                                  scene->visible_polygons_count);

    for (int i = 0; i < scene->visible_polygons_count; i++) {
        GamePolygon *polygon = scene->visible_polygons[i];
        GameModel *game_model = polygon->model;
        int face = polygon->face;

        if (game_model == scene->view) {
            scene_render_polygon_2d_face(scene, face);
        } else {
            int plane_index = 0;
            int vertex_shade = 0;
            uint16_t face_vertex_count = game_model->face_vertex_count[face];
            uint16_t *face_vertices = game_model->face_vertices[face];

            if (game_model->face_intensity[face] != GAME_MODEL_USE_GOURAUD) {
                if (polygon->visibility < 0) {
                    vertex_shade = game_model->light_ambience -
                                   game_model->face_intensity[face];
                } else {
                    vertex_shade = game_model->light_ambience +
                                   game_model->face_intensity[face];
                }
            }

            for (int j = 0; j < face_vertex_count; j++) {
                uint16_t vertex_index = face_vertices[j];

                scene->vertex_x[j] = game_model->project_vertex_x[vertex_index];
                scene->vertex_y[j] = game_model->project_vertex_y[vertex_index];
                scene->vertex_z[j] = game_model->project_vertex_z[vertex_index];

                if (game_model->face_intensity[face] ==
                    GAME_MODEL_USE_GOURAUD) {
                    if (polygon->visibility < 0) {
                        vertex_shade =
                            game_model->light_ambience -
                            game_model->vertex_intensity[vertex_index] +
                            game_model->vertex_ambience[vertex_index];
                    } else {
                        vertex_shade =
                            game_model->light_ambience +
                            game_model->vertex_intensity[vertex_index] +
                            game_model->vertex_ambience[vertex_index];
                    }
                }

                if (game_model->project_vertex_z[vertex_index] >=
                    scene->clip_near) {
                    scene->plane_x[plane_index] =
                        game_model->vertex_view_x[vertex_index];

                    scene->plane_y[plane_index] =
                        game_model->vertex_view_y[vertex_index];

                    scene->vertex_shade[plane_index] = vertex_shade;

                    if (game_model->project_vertex_z[vertex_index] >
                        scene->fog_z_distance) {
                        scene->vertex_shade[plane_index] +=
                            (game_model->project_vertex_z[vertex_index] -
                             scene->fog_z_distance);
                    }

                    plane_index++;
                } else {
                    uint16_t previous_vertex_index = 0;

                    if (j == 0) {
                        previous_vertex_index =
                            face_vertices[face_vertex_count - 1];
                    } else {
                        previous_vertex_index = face_vertices[j - 1];
                    }

                    if (game_model->project_vertex_z[previous_vertex_index] >=
                        scene->clip_near) {
                        int k7 =
                            game_model->project_vertex_z[vertex_index] -
                            game_model->project_vertex_z[previous_vertex_index];

                        int i5 =
                            game_model->project_vertex_x[vertex_index] -
                            ((game_model->project_vertex_x[vertex_index] -
                              game_model
                                  ->project_vertex_x[previous_vertex_index]) *
                             (game_model->project_vertex_z[vertex_index] -
                              scene->clip_near)) /
                                k7;

                        int j6 =
                            game_model->project_vertex_y[vertex_index] -
                            ((game_model->project_vertex_y[vertex_index] -
                              game_model
                                  ->project_vertex_y[previous_vertex_index]) *
                             (game_model->project_vertex_z[vertex_index] -
                              scene->clip_near)) /
                                k7;

                        scene->plane_x[plane_index] =
                            //(i5 << scene->view_distance) / scene->clip_near;
                            (i5 * scene->view_distance) / scene->clip_near;

                        scene->plane_y[plane_index] =
                            //(j6 << scene->view_distance) / scene->clip_near;
                            (j6 * scene->view_distance) / scene->clip_near;

                        scene->vertex_shade[plane_index] = vertex_shade;
                        plane_index++;
                    }

                    if (j == face_vertex_count - 1) {
                        previous_vertex_index = face_vertices[0];
                    } else {
                        previous_vertex_index = face_vertices[j + 1];
                    }

                    if (game_model->project_vertex_z[previous_vertex_index] >=
                        scene->clip_near) {
                        int l7 =
                            game_model->project_vertex_z[vertex_index] -
                            game_model->project_vertex_z[previous_vertex_index];

                        int j5 =
                            game_model->project_vertex_x[vertex_index] -
                            ((game_model
                                  ->project_vertex_x[previous_vertex_index] -
                              game_model
                                  ->project_vertex_x[previous_vertex_index]) *
                             (game_model
                                  ->project_vertex_z[previous_vertex_index] -
                              scene->clip_near)) /
                                l7;

                        int k6 =
                            game_model
                                ->project_vertex_y[previous_vertex_index] -
                            ((game_model
                                  ->project_vertex_y[previous_vertex_index] -
                              game_model
                                  ->project_vertex_y[previous_vertex_index]) *
                             (game_model
                                  ->project_vertex_z[previous_vertex_index] -
                              scene->clip_near)) /
                                l7;

                        scene->plane_x[plane_index] =
                            //(j5 << scene->view_distance) / scene->clip_near;
                            (j5 * scene->view_distance) / scene->clip_near;

                        scene->plane_y[plane_index] =
                            //(k6 << scene->view_distance) / scene->clip_near;
                            (k6 * scene->view_distance) / scene->clip_near;

                        scene->vertex_shade[plane_index] = vertex_shade;
                        plane_index++;
                    }
                }
            }

            for (int j = 0; j < face_vertex_count; j++) {
                if (scene->vertex_shade[j] < 0) {
                    scene->vertex_shade[j] = 0;
                } else if (scene->vertex_shade[j] > 255) {
                    scene->vertex_shade[j] = 255;
                }

                if (polygon->facefill >= 0) {
                    if (scene->texture_dimension[polygon->facefill] == 1) {
                        scene->vertex_shade[j] <<= 9; // * 512
                    } else {
                        scene->vertex_shade[j] <<= 6; // * 64
                    }
                }
            }

            scene_generate_scanlines(scene, plane_index, scene->plane_x,
                                     scene->plane_y, scene->vertex_shade,
                                     game_model, face);

            if (scene->max_y > scene->min_y) {
                scene_rasterize(scene, face_vertex_count, scene->vertex_x,
                                scene->vertex_y, scene->vertex_z,
                                polygon->facefill, game_model);
            }
        }
    }
#elif defined(RENDER_GL)
    scene_gl_render(scene);
#elif defined(RENDER_3DS_GL)
    scene_3ds_gl_render(scene);
#endif
    scene->mouse_picking_active = 0;
}

#ifdef RENDER_SW
static void scene_generate_scanlines(Scene *scene, int plane, int32_t *plane_x,
                                     int32_t *plane_y, int32_t *vertex_shade,
                                     GameModel *game_model, int face) {
    int start_x = 0;
    int end_x = 0;
    int start_s = 0;

    if (plane == 3) {
        int plane_y_0 = plane_y[0] + scene->base_y;
        int plane_y_1 = plane_y[1] + scene->base_y;
        int plane_y_2 = plane_y[2] + scene->base_y;
        int plane_x_0 = plane_x[0];
        int plane_x_1 = plane_x[1];
        int plane_x_2 = plane_x[2];
        int vertex_shade_0 = vertex_shade[0];
        int vertex_shade_1 = vertex_shade[1];
        int vertex_shade_2 = vertex_shade[2];
        int j12 = scene->base_y + scene->clip_y - 1;

        int l12 = 0;
        int j13 = 0;
        int l13 = 0;
        int j14 = 0;
        int l14 = 12345678;
        int j15 = -12345678;

        if (plane_y_2 != plane_y_0) {
            j13 = ((plane_x_2 - plane_x_0) << 8) / (plane_y_2 - plane_y_0);

            j14 = ((vertex_shade_2 - vertex_shade_0) << 8) /
                  (plane_y_2 - plane_y_0);

            if (plane_y_0 < plane_y_2) {
                l12 = plane_x_0 << 8;
                l13 = vertex_shade_0 << 8;
                l14 = plane_y_0;
                j15 = plane_y_2;
            } else {
                l12 = plane_x_2 << 8;
                l13 = vertex_shade_2 << 8;
                l14 = plane_y_2;
                j15 = plane_y_0;
            }

            if (l14 < 0) {
                l12 -= j13 * l14;
                l13 -= j14 * l14;
                l14 = 0;
            }

            if (j15 > j12) {
                j15 = j12;
            }
        }

        int l15 = 0;
        int j16 = 0;
        int l16 = 0;
        int j17 = 0;
        int l17 = 12345678;
        int j18 = -12345678;

        if (plane_y_1 != plane_y_0) {
            j16 = ((plane_x_1 - plane_x_0) << 8) / (plane_y_1 - plane_y_0);

            j17 = ((vertex_shade_1 - vertex_shade_0) << 8) /
                  (plane_y_1 - plane_y_0);

            if (plane_y_0 < plane_y_1) {
                l15 = plane_x_0 << 8;
                l16 = vertex_shade_0 << 8;
                l17 = plane_y_0;
                j18 = plane_y_1;
            } else {
                l15 = plane_x_1 << 8;
                l16 = vertex_shade_1 << 8;
                l17 = plane_y_1;
                j18 = plane_y_0;
            }

            if (l17 < 0) {
                l15 -= j16 * l17;
                l16 -= j17 * l17;
                l17 = 0;
            }

            if (j18 > j12) {
                j18 = j12;
            }
        }

        int l18 = 0;
        int j19 = 0;
        int l19 = 0;
        int j20 = 0;
        int l20 = 12345678;
        int j21 = -12345678;

        if (plane_y_2 != plane_y_1) {
            j19 = ((plane_x_2 - plane_x_1) << 8) / (plane_y_2 - plane_y_1);

            j20 = ((vertex_shade_2 - vertex_shade_1) << 8) /
                  (plane_y_2 - plane_y_1);

            if (plane_y_1 < plane_y_2) {
                l18 = plane_x_1 << 8;
                l19 = vertex_shade_1 << 8;
                l20 = plane_y_1;
                j21 = plane_y_2;
            } else {
                l18 = plane_x_2 << 8;
                l19 = vertex_shade_2 << 8;
                l20 = plane_y_2;
                j21 = plane_y_1;
            }

            if (l20 < 0) {
                l18 -= j19 * l20;
                l19 -= j20 * l20;
                l20 = 0;
            }

            if (j21 > j12) {
                j21 = j12;
            }
        }

        scene->min_y = l14;

        if (l17 < scene->min_y) {
            scene->min_y = l17;
        }

        if (l20 < scene->min_y) {
            scene->min_y = l20;
        }

        scene->max_y = j15;

        if (j18 > scene->max_y) {
            scene->max_y = j18;
        }

        if (j21 > scene->max_y) {
            scene->max_y = j21;
        }

        int end_s = 0;

        for (int y = scene->min_y; y < scene->max_y; y++) {
            if (y >= l14 && y < j15) {
                start_x = end_x = l12;
                start_s = end_s = l13;
                l12 += j13;
                l13 += j14;
            } else {
                start_x = 655360;
                end_x = -655360;
            }

            if (y >= l17 && y < j18) {
                if (l15 < start_x) {
                    start_x = l15;
                    start_s = l16;
                }

                if (l15 > end_x) {
                    end_x = l15;
                    end_s = l16;
                }

                l15 += j16;
                l16 += j17;
            }

            if (y >= l20 && y < j21) {
                if (l18 < start_x) {
                    start_x = l18;
                    start_s = l19;
                }

                if (l18 > end_x) {
                    end_x = l18;
                    end_s = l19;
                }

                l18 += j19;
                l19 += j20;
            }

            Scanline *scanline_6 = &scene->scanlines[y];
            scanline_6->start_x = start_x;
            scanline_6->end_x = end_x;
            scanline_6->start_s = start_s;
            scanline_6->end_s = end_s;
        }

        if (scene->min_y < scene->base_y - scene->clip_y) {
            scene->min_y = scene->base_y - scene->clip_y;
        }
    } else if (plane == 4) {
        int plane_y_0 = plane_y[0] + scene->base_y;
        int plane_y_1 = plane_y[1] + scene->base_y;
        int plane_y_2 = plane_y[2] + scene->base_y;
        int plane_y_3 = plane_y[3] + scene->base_y;
        int plane_x_0 = plane_x[0];
        int plane_x_1 = plane_x[1];
        int plane_x_2 = plane_x[2];
        int plane_x_3 = plane_x[3];
        int vertex_shade_0 = vertex_shade[0];
        int vertex_shade_1 = vertex_shade[1];
        int vertex_shade_2 = vertex_shade[2];
        int vertex_shade_3 = vertex_shade[3];
        int i14 = scene->base_y + scene->clip_y - 1;

        int k14 = 0;
        int i15 = 0;
        int k15 = 0;
        int i16 = 0;
        int k16 = 12345678;
        int i17 = -12345678;

        if (plane_y_3 != plane_y_0) {
            i15 = ((plane_x_3 - plane_x_0) << 8) / (plane_y_3 - plane_y_0);
            i16 = ((vertex_shade_3 - vertex_shade_0) << 8) /
                  (plane_y_3 - plane_y_0);

            if (plane_y_0 < plane_y_3) {
                k14 = plane_x_0 << 8;
                k15 = vertex_shade_0 << 8;
                k16 = plane_y_0;
                i17 = plane_y_3;
            } else {
                k14 = plane_x_3 << 8;
                k15 = vertex_shade_3 << 8;
                k16 = plane_y_3;
                i17 = plane_y_0;
            }

            if (k16 < 0) {
                k14 -= i15 * k16;
                k15 -= i16 * k16;
                k16 = 0;
            }

            if (i17 > i14) {
                i17 = i14;
            }
        }

        int k17 = 0;
        int i18 = 0;
        int k18 = 0;
        int i19 = 0;
        int k19 = 12345678;
        int i20 = -12345678;

        if (plane_y_1 != plane_y_0) {
            i18 = ((plane_x_1 - plane_x_0) << 8) / (plane_y_1 - plane_y_0);
            i19 = ((vertex_shade_1 - vertex_shade_0) << 8) /
                  (plane_y_1 - plane_y_0);

            if (plane_y_0 < plane_y_1) {
                k17 = plane_x_0 << 8;
                k18 = vertex_shade_0 << 8;
                k19 = plane_y_0;
                i20 = plane_y_1;
            } else {
                k17 = plane_x_1 << 8;
                k18 = vertex_shade_1 << 8;
                k19 = plane_y_1;
                i20 = plane_y_0;
            }

            if (k19 < 0) {
                k17 -= i18 * k19;
                k18 -= i19 * k19;
                k19 = 0;
            }

            if (i20 > i14) {
                i20 = i14;
            }
        }

        int k20 = 0;
        int i21 = 0;
        int k21 = 0;
        int i22 = 0;
        int j22 = 12345678;
        int k22 = -12345678;

        if (plane_y_2 != plane_y_1) {
            i21 = ((plane_x_2 - plane_x_1) << 8) / (plane_y_2 - plane_y_1);
            i22 = ((vertex_shade_2 - vertex_shade_1) << 8) /
                  (plane_y_2 - plane_y_1);

            if (plane_y_1 < plane_y_2) {
                k20 = plane_x_1 << 8;
                k21 = vertex_shade_1 << 8;
                j22 = plane_y_1;
                k22 = plane_y_2;
            } else {
                k20 = plane_x_2 << 8;
                k21 = vertex_shade_2 << 8;
                j22 = plane_y_2;
                k22 = plane_y_1;
            }

            if (j22 < 0) {
                k20 -= i21 * j22;
                k21 -= i22 * j22;
                j22 = 0;
            }

            if (k22 > i14) {
                k22 = i14;
            }
        }

        int l22 = 0;
        int i23 = 0;
        int j23 = 0;
        int k23 = 0;
        int l23 = 12345678;
        int i24 = -12345678;

        if (plane_y_3 != plane_y_2) {
            i23 = ((plane_x_3 - plane_x_2) << 8) / (plane_y_3 - plane_y_2);
            k23 = ((vertex_shade_3 - vertex_shade_2) << 8) /
                  (plane_y_3 - plane_y_2);

            if (plane_y_2 < plane_y_3) {
                l22 = plane_x_2 << 8;
                j23 = vertex_shade_2 << 8;
                l23 = plane_y_2;
                i24 = plane_y_3;
            } else {
                l22 = plane_x_3 << 8;
                j23 = vertex_shade_3 << 8;
                l23 = plane_y_3;
                i24 = plane_y_2;
            }

            if (l23 < 0) {
                l22 -= i23 * l23;
                j23 -= k23 * l23;
                l23 = 0;
            }

            if (i24 > i14) {
                i24 = i14;
            }
        }

        scene->min_y = k16;

        if (k19 < scene->min_y) {
            scene->min_y = k19;
        }

        if (j22 < scene->min_y) {
            scene->min_y = j22;
        }

        if (l23 < scene->min_y) {
            scene->min_y = l23;
        }

        scene->max_y = i17;

        if (i20 > scene->max_y) {
            scene->max_y = i20;
        }

        if (k22 > scene->max_y) {
            scene->max_y = k22;
        }

        if (i24 > scene->max_y) {
            scene->max_y = i24;
        }

        int end_s = 0;

        for (int y = scene->min_y; y < scene->max_y; y++) {
            if (y >= k16 && y < i17) {
                start_x = end_x = k14;
                start_s = end_s = k15;
                k14 += i15;
                k15 += i16;
            } else {
                start_x = 655360;
                end_x = -655360;
            }

            if (y >= k19 && y < i20) {
                if (k17 < start_x) {
                    start_x = k17;
                    start_s = k18;
                }

                if (k17 > end_x) {
                    end_x = k17;
                    end_s = k18;
                }

                k17 += i18;
                k18 += i19;
            }

            if (y >= j22 && y < k22) {
                if (k20 < start_x) {
                    start_x = k20;
                    start_s = k21;
                }

                if (k20 > end_x) {
                    end_x = k20;
                    end_s = k21;
                }

                k20 += i21;
                k21 += i22;
            }

            if (y >= l23 && y < i24) {
                if (l22 < start_x) {
                    start_x = l22;
                    start_s = j23;
                }

                if (l22 > end_x) {
                    end_x = l22;
                    end_s = j23;
                }

                l22 += i23;
                j23 += k23;
            }

            Scanline *scanline_7 = &scene->scanlines[y];
            scanline_7->start_x = start_x;
            scanline_7->end_x = end_x;
            scanline_7->start_s = start_s;
            scanline_7->end_s = end_s;
        }

        if (scene->min_y < scene->base_y - scene->clip_y) {
            scene->min_y = scene->base_y - scene->clip_y;
        }
    } else {
        scene->max_y = scene->min_y = plane_y[0] += scene->base_y;

        for (int y = 1; y < plane; y++) {
            int i2 = 0;

            if ((i2 = plane_y[y] += scene->base_y) < scene->min_y) {
                scene->min_y = i2;
            } else if (i2 > scene->max_y) {
                scene->max_y = i2;
            }
        }

        if (scene->min_y < scene->base_y - scene->clip_y) {
            scene->min_y = scene->base_y - scene->clip_y;
        }

        if (scene->max_y >= scene->base_y + scene->clip_y) {
            scene->max_y = scene->base_y + scene->clip_y - 1;
        }

        if (scene->min_y >= scene->max_y) {
            return;
        }

        for (int y = scene->min_y; y < scene->max_y; y++) {
            Scanline *scanline = &scene->scanlines[y];
            scanline->start_x = 655360;
            scanline->end_x = -655360;
        }

        int j2 = plane - 1;
        int i3 = plane_y[0];
        int i4 = plane_y[j2];

        if (i3 < i4) {
            int i5 = plane_x[0] << 8;
            int j6 = ((plane_x[j2] - plane_x[0]) << 8) / (i4 - i3);
            int l7 = vertex_shade[0] << 8;
            int j9 = ((vertex_shade[j2] - vertex_shade[0]) << 8) / (i4 - i3);

            if (i3 < 0) {
                i5 -= j6 * i3;
                l7 -= j9 * i3;
                i3 = 0;
            }

            if (i4 > scene->max_y) {
                i4 = scene->max_y;
            }

            for (int y = i3; y <= i4; y++) {
                Scanline *scanline_2 = &scene->scanlines[y];
                scanline_2->start_x = scanline_2->end_x = i5;
                scanline_2->start_s = scanline_2->end_s = l7;
                i5 += j6;
                l7 += j9;
            }
        } else if (i3 > i4) {
            int j5 = plane_x[j2] << 8;
            int k6 = ((plane_x[0] - plane_x[j2]) << 8) / (i3 - i4);
            int i8 = vertex_shade[j2] << 8;
            int k9 = ((vertex_shade[0] - vertex_shade[j2]) << 8) / (i3 - i4);

            if (i4 < 0) {
                j5 -= k6 * i4;
                i8 -= k9 * i4;
                i4 = 0;
            }

            if (i3 > scene->max_y) {
                i3 = scene->max_y;
            }

            for (int y = i4; y <= i3; y++) {
                Scanline *scanline_3 = &scene->scanlines[y];
                scanline_3->start_x = j5;
                scanline_3->end_x = j5;
                scanline_3->start_s = i8;
                scanline_3->end_s = i8;
                j5 += k6;
                i8 += k9;
            }
        }

        for (int y = 0; y < j2; y++) {
            int k5 = y + 1;
            int j3 = plane_y[y];
            int j4 = plane_y[k5];

            if (j3 < j4) {
                int l6 = plane_x[y] << 8;
                int j8 = ((plane_x[k5] - plane_x[y]) << 8) / (j4 - j3);
                int l9 = vertex_shade[y] << 8;

                int l10 =
                    ((vertex_shade[k5] - vertex_shade[y]) << 8) / (j4 - j3);

                if (j3 < 0) {
                    l6 -= j8 * j3;
                    l9 -= l10 * j3;
                    j3 = 0;
                }

                if (j4 > scene->max_y) {
                    j4 = scene->max_y;
                }

                for (int l11 = j3; l11 <= j4; l11++) {
                    Scanline *scanline_4 = &scene->scanlines[l11];

                    if (l6 < scanline_4->start_x) {
                        scanline_4->start_x = l6;
                        scanline_4->start_s = l9;
                    }

                    if (l6 > scanline_4->end_x) {
                        scanline_4->end_x = l6;
                        scanline_4->end_s = l9;
                    }

                    l6 += j8;
                    l9 += l10;
                }
            } else if (j3 > j4) {
                int i7 = plane_x[k5] << 8;
                int k8 = ((plane_x[y] - plane_x[k5]) << 8) / (j3 - j4);
                int i10 = vertex_shade[k5] << 8;
                int i11 =
                    ((vertex_shade[y] - vertex_shade[k5]) << 8) / (j3 - j4);

                if (j4 < 0) {
                    i7 -= k8 * j4;
                    i10 -= i11 * j4;
                    j4 = 0;
                }

                if (j3 > scene->max_y) {
                    j3 = scene->max_y;
                }

                for (int i12 = j4; i12 <= j3; i12++) {
                    Scanline *scanline_5 = &scene->scanlines[i12];

                    if (i7 < scanline_5->start_x) {
                        scanline_5->start_x = i7;
                        scanline_5->start_s = i10;
                    }

                    if (i7 > scanline_5->end_x) {
                        scanline_5->end_x = i7;
                        scanline_5->end_s = i10;
                    }

                    i7 += k8;
                    i10 += i11;
                }
            }
        }

        if (scene->min_y < scene->base_y - scene->clip_y) {
            scene->min_y = scene->base_y - scene->clip_y;
        }
    }

    if (scene->mouse_picking_active && scene->mouse_y >= scene->min_y &&
        scene->mouse_y < scene->max_y) {
        Scanline *scanline_1 = &scene->scanlines[scene->mouse_y];

        if (scene->mouse_x >= scanline_1->start_x >> 8 &&
            scene->mouse_x <= scanline_1->end_x >> 8 &&
            scanline_1->start_x <= scanline_1->end_x &&
            !game_model->unpickable && game_model->is_local_player[face] == 0) {

            scene_mouse_pick(scene, game_model, face);
        }
    }
}

static void scene_rasterize(Scene *scene, int vertex_count, int32_t *vertices_x,
                            int32_t *vertices_y, int32_t *vertices_z,
                            int face_fill, GameModel *game_model) {
#ifdef USE_TOONSCAPE
    face_fill = apply_toonscape(face_fill);
#endif

    if (face_fill == -2) {
        return;
    }

    /* face_fill >= 0 => textured; face_fill < 0 => color ramp */
    if (face_fill >= 0) {
        if (face_fill >= scene->texture_count) {
            face_fill = 0;
        }
        scene_prepare_texture(scene, face_fill);

        int vertex_x = vertices_x[0];
        int vertex_y = vertices_y[0];
        int vertex_z = vertices_z[0];
        int i3 = vertex_x - vertices_x[1];
        int k3 = vertex_y - vertices_y[1];
        int i4 = vertex_z - vertices_z[1];

        vertex_count--;

        int i6 = vertices_x[vertex_count] - vertex_x;
        int j7 = vertices_y[vertex_count] - vertex_y;
        int k8 = vertices_z[vertex_count] - vertex_z;

        /* Handle dimension == 1 or dimension == 2 differently */
        if (scene->texture_dimension[face_fill] == 1) {
            /* -------------- TEXTURE DIMENSION == 1 -------------- */
            int l9  = (i6 * vertex_y - j7 * vertex_x) << 12;
            int k10 = (j7 * vertex_z - k8 * vertex_y) << 7;
            int i11 = (k8 * vertex_x - i6 * vertex_z) << 3;

            int k11 = (i3 * vertex_y - k3 * vertex_x) << 12;
            int i12 = (k3 * vertex_z - i4 * vertex_y) << 7;
            int k12 = (i4 * vertex_x - i3 * vertex_z) << 3;

            int i13 = (k3 * i6 - i3 * j7) << 5;
            int k13 = (i4 * j7 - k3 * k8);
            int i14 = (i3 * k8 - i4 * i6) >> 4;

            int k14 = k10 >> 4;
            int i15 = i12 >> 4;
            int k15 = k13 >> 4;

            int i16 = scene->min_y - scene->base_y;
            int k16 = scene->width;
            int i17 = scene->base_x + scene->min_y * k16;
            int8_t scanline_inc = 1;

            /* Offset l9,k11,i13 by the portion that min_y has advanced. */
            l9  += i11 * i16;
            k11 += k12 * i16;
            i13 += i14 * i16;

            if (scene->interlace) {
                /* If odd line, skip one to align with even lines. */
                if ((scene->min_y & 1) == 1) {
                    scene->min_y++;
                    l9  += i11;
                    k11 += k12;
                    i13 += i14;
                    i17 += k16;
                }
                /* Double stepping vertically. */
                i11 <<= 1;
                k12 <<= 1;
                i14 <<= 1;
                k16 <<= 1;
                scanline_inc = 2;
            }

            if (!scene->texture_back_transparent[face_fill]) {
                /* Non-alpha KEY path */
                for (int y = scene->min_y; y < scene->max_y; y += scanline_inc) {
                    Scanline *scanline = &scene->scanlines[y];
                    int j = scanline->start_x >> 8;
                    int i18 = scanline->end_x >> 8;
                    int length = i18 - j;

                    if (length <= 0) {
                        /* Advance param increments and skip. */
                        l9  += i11;
                        k11 += k12;
                        i13 += i14;
                        i17 += k16;
                        continue;
                    }

                    /* Compute start_s and slope in screen-space */
                    int j22 = scanline->start_s;
                    int l23 = (scanline->end_s - j22) / length;

                    /* Clipping (left) */
                    if (j < -scene->clip_x) {
                        int diff = -scene->clip_x - j;
                        j22 += diff * l23;
                        j = -scene->clip_x;
                    }
                    /* Clipping (right) */
                    if (i18 > scene->clip_x) {
                        i18 = scene->clip_x;
                    }
                    /* Recompute length after clipping */
                    length = i18 - j;
                    if (length > 0) {
                        scene_texture128_scanline(
                            scene->raster + (i17 + j),
                            scene->texture_pixels[face_fill],
                            (l9 + k14 * j), (k11 + i15 * j), (i13 + k15 * j),
                            k10, i12, k13, length, j22, (l23 << 2));
                    }

                    /* Advance for next scanline */
                    l9  += i11;
                    k11 += k12;
                    i13 += i14;
                    i17 += k16;
                }
                return;
            }

            /* Alpha-KEY path */
            for (int y = scene->min_y; y < scene->max_y; y += scanline_inc) {
                Scanline *scanline = &scene->scanlines[y];
                int j = scanline->start_x >> 8;
                int k18 = scanline->end_x >> 8;
                int length = k18 - j;

                if (length <= 0) {
                    l9  += i11;
                    k11 += k12;
                    i13 += i14;
                    i17 += k16;
                    continue;
                }

                int k22 = scanline->start_s;
                int i24 = (scanline->end_s - k22) / length;

                if (j < -scene->clip_x) {
                    int diff = -scene->clip_x - j;
                    k22 += diff * i24;
                    j = -scene->clip_x;
                }
                if (k18 > scene->clip_x) {
                    k18 = scene->clip_x;
                }

                length = k18 - j;
                if (length > 0) {
                    scene_texture128_alphakey_scanline(
                        scene->raster + (i17 + j),
                        scene->texture_pixels[face_fill],
                        (l9 + k14 * j), (k11 + i15 * j), (i13 + k15 * j),
                        k10, i12, k13, length, k22, i24);
                }

                l9  += i11;
                k11 += k12;
                i13 += i14;
                i17 += k16;
            }
            return;
        }

        /* -------------- TEXTURE DIMENSION == 2 -------------- */
        int i10 = (i6 * vertex_y - j7 * vertex_x) << 11;
        int l10 = (j7 * vertex_z - k8 * vertex_y) << 6;
        int j11 = (k8 * vertex_x - i6 * vertex_z) << 2;

        int l11 = (i3 * vertex_y - k3 * vertex_x) << 11;
        int j12 = (k3 * vertex_z - i4 * vertex_y) << 6;
        int l12 = (i4 * vertex_x - i3 * vertex_z) << 2;

        int j13 = (k3 * i6 - i3 * j7) << 5;
        int l13 = (i4 * j7 - k3 * k8);
        int j14 = (i3 * k8 - i4 * i6) >> 4;

        int l14 = l10 >> 4;
        int j15 = j12 >> 4;
        int l15 = l13 >> 4;

        int j16 = scene->min_y - scene->base_y;
        int l16 = scene->width;
        int j17 = scene->base_x + scene->min_y * l16;
        int8_t scanline_inc = 1;

        i10 += j11 * j16;
        l11 += l12 * j16;
        j13 += j14 * j16;

        if (scene->interlace) {
            if ((scene->min_y & 1) == 1) {
                scene->min_y++;
                i10  += j11;
                l11  += l12;
                j13  += j14;
                j17  += l16;
            }
            j11 <<= 1;
            l12 <<= 1;
            j14 <<= 1;
            l16 <<= 1;
            scanline_inc = 2;
        }

        if (!scene->texture_back_transparent[face_fill]) {
            /* Non-alpha KEY path */
            for (int y = scene->min_y; y < scene->max_y; y += scanline_inc) {
                Scanline *scanline = &scene->scanlines[y];
                int j = scanline->start_x >> 8;
                int k19 = scanline->end_x >> 8;
                int length = k19 - j;

                if (length <= 0) {
                    i10  += j11;
                    l11  += l12;
                    j13  += j14;
                    j17  += l16;
                    continue;
                }

                int i23 = scanline->start_s;
                int k24 = (scanline->end_s - i23) / length;

                if (j < -scene->clip_x) {
                    int diff = -scene->clip_x - j;
                    i23 += diff * k24;
                    j += diff;
                }
                if (k19 > scene->clip_x) {
                    k19 = scene->clip_x;
                }
                length = k19 - j;
                if (length > 0) {
                    scene_texture64_scanline(
                        scene->raster + (j17 + j),
                        scene->texture_pixels[face_fill],
                        (i10 + l14 * j), (l11 + j15 * j), (j13 + l15 * j),
                        l10, j12, l13, length, i23, k24);
                }

                i10  += j11;
                l11  += l12;
                j13  += j14;
                j17  += l16;
            }
            return;
        }

        /* Alpha-KEY path */
        for (int y = scene->min_y; y < scene->max_y; y += scanline_inc) {
            Scanline *scanline = &scene->scanlines[y];
            int j = scanline->start_x >> 8;
            int i20 = scanline->end_x >> 8;
            int l21 = i20 - j;

            if (l21 <= 0) {
                i10  += j11;
                l11  += l12;
                j13  += j14;
                j17  += l16;
                continue;
            }

            int j23 = scanline->start_s;
            int l24 = (scanline->end_s - j23) / l21;

            if (j < -scene->clip_x) {
                int diff = -scene->clip_x - j;
                j23 += diff * l24;
                j += diff;
            }
            if (i20 > scene->clip_x) {
                i20 = scene->clip_x;
            }
            l21 = i20 - j;
            if (l21 > 0) {
                scene_texture64_alphakey_scanline(
                    scene->raster + (j17 + j),
                    scene->texture_pixels[face_fill],
                    (i10 + l14 * j), (l11 + j15 * j), (j13 + l15 * j),
                    l10, j12, l13, l21, j23, l24);
            }

            i10  += j11;
            l11  += l12;
            j13  += j14;
            j17  += l16;
        }
        return;
    }

    /* -------------- COLORED (face_fill < 0) -------------- */
    for (int i = 0; i < RAMP_COUNT; i++) {
        if (scene->gradient_base[i] == face_fill) {
            scene->gradient_ramp = scene->gradient_ramps[i];
            break;
        }
        if (i == RAMP_COUNT - 1) {
            int gradient_index =
                (int)(((float)rand() / (float)RAND_MAX) * RAMP_COUNT);

            scene->gradient_base[gradient_index] = face_fill;
            face_fill = -1 - face_fill;

            int r = ((face_fill >> 10) & 0x1f) * 8;
            int g = ((face_fill >> 5) & 0x1f) * 8;
            int b = (face_fill & 0x1f) * 8;

            for (int j = 0; j < 256; j++) {
                int darkness = j * j;
                int dark_r = (r * darkness) / 0x10000;
                int dark_g = (g * darkness) / 0x10000;
                int dark_b = (b * darkness) / 0x10000;

                scene->gradient_ramps[gradient_index][(RAMP_SIZE - 1) - j] =
                    (dark_r << 16) + (dark_g << 8) + dark_b;
            }
            scene->gradient_ramp = scene->gradient_ramps[gradient_index];
        }
    }

    int i2 = scene->width;
    int l2 = scene->base_x + scene->min_y * i2;
    int8_t scanline_inc = 1;

    if (scene->interlace) {
        if ((scene->min_y & 1) == 1) {
            scene->min_y++;
            l2 += i2;
        }
        i2 <<= 1;      /* double stepping horizontally */
        scanline_inc = 2;
    }

    if (game_model->transparent) {
        /* -------------- TRANSLUCENT RAMP FILL -------------- */
        for (int y = scene->min_y; y < scene->max_y; y += scanline_inc) {
            Scanline *scanline = &scene->scanlines[y];
            int j   = scanline->start_x >> 8;
            int k4  = scanline->end_x >> 8;
            int k6  = k4 - j;

            if (k6 <= 0) {
                l2 += i2;
                continue;
            }

            int ramp_index = scanline->start_s;
            int ramp_inc   = (scanline->end_s - ramp_index) / k6;

            /* Left clip */
            if (j < -scene->clip_x) {
                int diff = -scene->clip_x - j;
                ramp_index += diff * ramp_inc;
                j += diff;
            }
            /* Right clip */
            if (k4 > scene->clip_x) {
                k4 = scene->clip_x;
            }
            k6 = k4 - j;
            if (k6 > 0) {
                scene_colour_translucent_scanline(
                    scene->raster + (l2 + j), -k6, scene->gradient_ramp,
                    ramp_index, ramp_inc);
            }

            l2 += i2;
        }
        return;
    }

    /* -------------- OPAQUE RAMP FILL -------------- */
    for (int y = scene->min_y; y < scene->max_y; y += scanline_inc) {
        Scanline *scanline = &scene->scanlines[y];
        int j  = scanline->start_x >> 8;
        int k5 = scanline->end_x >> 8;
        int i7 = k5 - j;

        if (i7 <= 0) {
            l2 += i2;
            continue;
        }

        int ramp_index = scanline->start_s;
        int ramp_inc   = (scanline->end_s - ramp_index) / i7;

        if (j < -scene->clip_x) {
            int diff = -scene->clip_x - j;
            ramp_index += diff * ramp_inc;
            j += diff;
        }
        if (k5 > scene->clip_x) {
            k5 = scene->clip_x;
        }
        i7 = k5 - j;
        if (i7 > 0) {
            scene_colour_scanline(scene->raster + (l2 + j), -i7,
                                  scene->gradient_ramp, ramp_index, ramp_inc);
        }
        l2 += i2;
    }
}
#endif /* RENDER_SW */


void scene_set_camera(Scene *scene, int x, int y, int z, int yaw, int pitch,
                      int roll, int distance) {
    yaw &= 1023;
    pitch &= 1023;
    roll &= 1023;

    scene->camera_yaw = (1024 - yaw) & 1023;
    scene->camera_pitch = (1024 - pitch) & 1023;
    scene->camera_roll = (1024 - roll) & 1023;

    int offset_x = 0;
    int offset_y = 0;
    int offset_z = distance;

    if (yaw != 0) {
        int sine = sin_cos_2048[yaw];
        int cosine = sin_cos_2048[yaw + 1024];
        int i4 = (offset_y * cosine - offset_z * sine) >> 15;
        offset_z = (offset_y * sine + offset_z * cosine) >> 15;
        offset_y = i4;
    }

    if (pitch != 0) {
        int sine = sin_cos_2048[pitch];
        int cosine = sin_cos_2048[pitch + 1024];
        int j4 = (offset_z * sine + offset_x * cosine) >> 15;
        offset_z = (offset_z * cosine - offset_x * sine) >> 15;
        offset_x = j4;
    }

    if (roll != 0) {
        int sine = sin_cos_2048[roll];
        int cosine = sin_cos_2048[roll + 1024];
        int k4 = (offset_y * sine + offset_x * cosine) >> 15;
        offset_y = (offset_y * cosine - offset_x * sine) >> 15;
        offset_x = k4;
    }

    scene->camera_x = x - offset_x;
    scene->camera_y = y - offset_y;
    scene->camera_z = z - offset_z;

#if defined(RENDER_GL) || defined(RENDER_3DS_GL)
    scene_gl_update_camera(scene);
#endif
}

#ifdef RENDER_SW
static void scene_initialise_polygon_3d(Scene *scene, int polygon_index) {
    GamePolygon *polygon = scene->visible_polygons[polygon_index];
    GameModel *game_model = polygon->model;
    int face = polygon->face;

    uint16_t *fv = game_model->face_vertices[face]; // indices of vertices
    uint8_t face_vertex_count = game_model->face_vertex_count[face];
    int normal_scale = game_model->normal_scale[face];

    int16_t *px = game_model->project_vertex_x;
    int16_t *py = game_model->project_vertex_y;
    int16_t *pz = game_model->project_vertex_z;

    int32_t *vx = game_model->vertex_view_x;
    int32_t *vy = game_model->vertex_view_y;

    int project_x_a = (int)px[fv[0]];
    int project_y_a = (int)py[fv[0]];
    int project_z_a = (int)pz[fv[0]];

    int project_x_b = (int)px[fv[1]];
    int project_y_b = (int)py[fv[1]];
    int project_z_b = (int)pz[fv[1]];

    int project_x_c = (int)px[fv[2]];
    int project_y_c = (int)py[fv[2]];
    int project_z_c = (int)pz[fv[2]];

    int project_x_delta_ba = project_x_b - project_x_a;
    int project_y_delta_ba = project_y_b - project_y_a;
    int project_z_delta_ba = project_z_b - project_z_a;

    int project_x_delta_ca = project_x_c - project_x_a;
    int project_y_delta_ca = project_y_c - project_y_a;
    int project_z_delta_ca = project_z_c - project_z_a;

    int normal_x = project_y_delta_ba * project_z_delta_ca
                 - project_y_delta_ca * project_z_delta_ba;
    int normal_y = project_z_delta_ba * project_x_delta_ca
                 - project_z_delta_ca * project_x_delta_ba;
    int normal_z = project_x_delta_ba * project_y_delta_ca
                 - project_x_delta_ca * project_y_delta_ba;

    if (normal_scale == -1) {
        int max_abs = normal_x;
        if (abs(normal_y) > abs(max_abs)) max_abs = normal_y;
        if (abs(normal_z) > abs(max_abs)) max_abs = normal_z;
        int abs_max = abs(max_abs);

        int shift = 0;
        while (abs_max > 25000) {
            abs_max >>= 1;
            shift++;
        }
        normal_scale = shift;
        game_model->normal_scale[face] = normal_scale;

        if (shift > 0) {
            normal_x >>= shift;
            normal_y >>= shift;
            normal_z >>= shift;
        }

        float len = sqrtf((float)(normal_x * (long long)normal_x +
                                  normal_y * (long long)normal_y +
                                  normal_z * (long long)normal_z));
        game_model->normal_magnitude[face] = scene->normal_magnitude * len;
    } else {
        normal_x >>= normal_scale;
        normal_y >>= normal_scale;
        normal_z >>= normal_scale;
    }

    polygon->visibility = project_x_a * normal_x
                        + project_y_a * normal_y
                        + project_z_a * normal_z;

    polygon->normal_x = normal_x;
    polygon->normal_y = normal_y;
    polygon->normal_z = normal_z;

    int min_z = (int)pz[fv[0]];
    int max_z = min_z;

    int min_plane_x = (int)vx[fv[0]];
    int max_plane_x = min_plane_x;

    int min_plane_y = (int)vy[fv[0]];
    int max_plane_y = min_plane_y;

    for (int i = 1; i < face_vertex_count; i++) {
        int idx = fv[i];

        int z_val = (int)pz[idx];
        if (z_val < min_z) {
            min_z = z_val;
        } else if (z_val > max_z) {
            max_z = z_val;
        }

        int x_val = (int)vx[idx];
        if (x_val < min_plane_x) {
            min_plane_x = x_val;
        } else if (x_val > max_plane_x) {
            max_plane_x = x_val;
        }

        int y_val = (int)vy[idx];
        if (y_val < min_plane_y) {
            min_plane_y = y_val;
        } else if (y_val > max_plane_y) {
            max_plane_y = y_val;
        }
    }

    polygon->min_z = min_z;
    polygon->max_z = max_z;
    polygon->min_plane_x = min_plane_x;
    polygon->max_plane_x = max_plane_x;
    polygon->min_plane_y = min_plane_y;
    polygon->max_plane_y = max_plane_y;
}

static int scene_separate_polygon(GamePolygon *polygon_a,
                                  GamePolygon *polygon_b) {

    if ((polygon_a->min_plane_x >= polygon_b->max_plane_x) ||
        (polygon_b->min_plane_x >= polygon_a->max_plane_x) ||
        (polygon_a->min_plane_y >= polygon_b->max_plane_y) ||
        (polygon_b->min_plane_y >= polygon_a->max_plane_y) ||
        (polygon_a->min_z >= polygon_b->max_z)) {
        return 1; /* No overlap */
    }
    if (polygon_b->min_z > polygon_a->max_z) {
        return 0; /* Overlap is determined false by z-plane alone */
    }

    GameModel *game_model_a = polygon_a->model;
    GameModel *game_model_b = polygon_b->model;

    int face_a = polygon_a->face;
    int face_b = polygon_b->face;

    uint16_t *face_vertices_a = game_model_a->face_vertices[face_a];
    uint16_t *face_vertices_b = game_model_b->face_vertices[face_b];

    int face_vertex_count_a = game_model_a->face_vertex_count[face_a];
    int face_vertex_count_b = game_model_b->face_vertex_count[face_b];

    int16_t *px_a = game_model_a->project_vertex_x;
    int16_t *py_a = game_model_a->project_vertex_y;
    int16_t *pz_a = game_model_a->project_vertex_z;

    int32_t *vx_a = game_model_a->vertex_view_x;
    int32_t *vy_a = game_model_a->vertex_view_y;

    int16_t *px_b = game_model_b->project_vertex_x;
    int16_t *py_b = game_model_b->project_vertex_y;
    int16_t *pz_b = game_model_b->project_vertex_z;

    int32_t *vx_b = game_model_b->vertex_view_x;
    int32_t *vy_b = game_model_b->vertex_view_y;

    int first_project_x = (int)px_b[face_vertices_b[0]];
    int first_project_y = (int)py_b[face_vertices_b[0]];
    int first_project_z = (int)pz_b[face_vertices_b[0]];

    int normal_x = polygon_b->normal_x;
    int normal_y = polygon_b->normal_y;
    int normal_z = polygon_b->normal_z;
    int normal_magnitude = game_model_b->normal_magnitude[face_b];
    int visibility = polygon_b->visibility;

    int flag = 0;  /* 0 means "not separated yet" */

    for (int i = 0; i < face_vertex_count_a; i++) {
        int idx = face_vertices_a[i];

        int dx = first_project_x - (int)px_a[idx];
        int dy = first_project_y - (int)py_a[idx];
        int dz = first_project_z - (int)pz_a[idx];

        int magnitude = dx * normal_x + dy * normal_y + dz * normal_z;

        int cond1 = (magnitude >= -normal_magnitude || visibility >= 0);
        int cond2 = (magnitude <=  normal_magnitude || visibility <= 0);
        if (! (cond1 && cond2)) {
            flag = 1;
            break;
        }
    }

    if (!flag) {
        return 1;
    }

    first_project_x = (int)px_a[face_vertices_a[0]];
    first_project_y = (int)py_a[face_vertices_a[0]];
    first_project_z = (int)pz_a[face_vertices_a[0]];

    normal_x = polygon_a->normal_x;
    normal_y = polygon_a->normal_y;
    normal_z = polygon_a->normal_z;
    normal_magnitude = game_model_a->normal_magnitude[face_a];
    visibility = polygon_a->visibility;

    flag = 0;

    for (int i = 0; i < face_vertex_count_b; i++) {
        int idx = face_vertices_b[i];

        int dx = first_project_x - (int)px_b[idx];
        int dy = first_project_y - (int)py_b[idx];
        int dz = first_project_z - (int)pz_b[idx];

        int magnitude = dx * normal_x + dy * normal_y + dz * normal_z;

        int cond1 = (magnitude >= -normal_magnitude || visibility <= 0);
        int cond2 = (magnitude <=  normal_magnitude || visibility >= 0);
        if (! (cond1 && cond2)) {
            flag = 1;
            break;
        }
    }

    if (!flag) {
        return 1;
    }

    int length_a;
    int *vertex_view_x_a = NULL;
    int *vertex_view_y_a = NULL;

    if (face_vertex_count_a == 2) {
        length_a = 4;
        vertex_view_x_a = alloca(length_a * sizeof(int));
        vertex_view_y_a = alloca(length_a * sizeof(int));

        int v0 = face_vertices_a[0];
        int v1 = face_vertices_a[1];

        int x0 = (int)vx_a[v0];
        int x1 = (int)vx_a[v1];
        int y0 = (int)vy_a[v0];
        int y1 = (int)vy_a[v1];

        vertex_view_x_a[0] = x0 - 20;
        vertex_view_x_a[1] = x1 - 20;
        vertex_view_x_a[2] = x1 + 20;
        vertex_view_x_a[3] = x0 + 20;

        vertex_view_y_a[0] = y0;
        vertex_view_y_a[1] = y1;
        vertex_view_y_a[2] = y1;
        vertex_view_y_a[3] = y0;
    } else {
        length_a = face_vertex_count_a;
        vertex_view_x_a = alloca(length_a * sizeof(int));
        vertex_view_y_a = alloca(length_a * sizeof(int));

        for (int i = 0; i < length_a; i++) {
            int idx = face_vertices_a[i];
            vertex_view_x_a[i] = (int)vx_a[idx];
            vertex_view_y_a[i] = (int)vy_a[idx];
        }
    }

    int length_b;
    int *vertex_view_x_b = NULL;
    int *vertex_view_y_b = NULL;

    if (face_vertex_count_b == 2) {
        length_b = 4;
        vertex_view_x_b = alloca(length_b * sizeof(int));
        vertex_view_y_b = alloca(length_b * sizeof(int));

        int v0 = face_vertices_b[0];
        int v1 = face_vertices_b[1];

        int x0 = (int)vx_b[v0];
        int x1 = (int)vx_b[v1];
        int y0 = (int)vy_b[v0];
        int y1 = (int)vy_b[v1];

        vertex_view_x_b[0] = x0 - 20;
        vertex_view_x_b[1] = x1 - 20;
        vertex_view_x_b[2] = x1 + 20;
        vertex_view_x_b[3] = x0 + 20;

        vertex_view_y_b[0] = y0;
        vertex_view_y_b[1] = y1;
        vertex_view_y_b[2] = y1;
        vertex_view_y_b[3] = y0;
    } else {
        length_b = face_vertex_count_b;
        vertex_view_x_b = alloca(length_b * sizeof(int));
        vertex_view_y_b = alloca(length_b * sizeof(int));

        for (int i = 0; i < length_b; i++) {
            int idx = face_vertices_b[i];
            vertex_view_x_b[i] = (int)vx_b[idx];
            vertex_view_y_b[i] = (int)vy_b[idx];
        }
    }

    return !scene_intersect(vertex_view_x_a, vertex_view_y_a,
                            vertex_view_x_b, vertex_view_y_b,
                            length_a, length_b);
}

static int scene_heuristic_polygon(GamePolygon *polygon_a,
                                   GamePolygon *polygon_b) {
    GameModel *game_model_a = polygon_a->model;
    GameModel *game_model_b = polygon_b->model;
    int face_a = polygon_a->face;
    int face_b = polygon_b->face;
    uint16_t *face_vertices_a = game_model_a->face_vertices[face_a];
    uint16_t *face_vertices_b = game_model_b->face_vertices[face_b];
    int face_vertex_count_a = game_model_a->face_vertex_count[face_a];
    int face_vertex_count_b = game_model_b->face_vertex_count[face_b];

    int first_project_x = game_model_b->project_vertex_x[face_vertices_b[0]];
    int first_project_y = game_model_b->project_vertex_y[face_vertices_b[0]];
    int first_project_z = game_model_b->project_vertex_z[face_vertices_b[0]];
    int normal_x = polygon_b->normal_x;
    int normal_y = polygon_b->normal_y;
    int normal_z = polygon_b->normal_z;
    int normal_magnitude = game_model_b->normal_magnitude[face_b];
    int visibility = polygon_b->visibility;
    int flag = 0;

    for (int i = 0; i < face_vertex_count_a; i++) {
        int vertex_index = face_vertices_a[i];

        int magnitude =
            (first_project_x - game_model_a->project_vertex_x[vertex_index]) *
                normal_x +
            (first_project_y - game_model_a->project_vertex_y[vertex_index]) *
                normal_y +
            (first_project_z - game_model_a->project_vertex_z[vertex_index]) *
                normal_z;

        if ((magnitude >= -normal_magnitude || visibility >= 0) &&
            (magnitude <= normal_magnitude || visibility <= 0)) {
            continue;
        }

        flag = 1;
        break;
    }

    if (!flag) {
        return 1;
    }

    first_project_x = game_model_a->project_vertex_x[face_vertices_a[0]];
    first_project_y = game_model_a->project_vertex_y[face_vertices_a[0]];
    first_project_z = game_model_a->project_vertex_z[face_vertices_a[0]];
    normal_x = polygon_a->normal_x;
    normal_y = polygon_a->normal_y;
    normal_z = polygon_a->normal_z;
    normal_magnitude = game_model_a->normal_magnitude[face_a];
    visibility = polygon_a->visibility;
    flag = 0;

    for (int i = 0; i < face_vertex_count_b; i++) {
        int vertex_index = face_vertices_b[i];

        int magnitude =
            (first_project_x - game_model_b->project_vertex_x[vertex_index]) *
                normal_x +
            (first_project_y - game_model_b->project_vertex_y[vertex_index]) *
                normal_y +
            (first_project_z - game_model_b->project_vertex_z[vertex_index]) *
                normal_z;

        if ((magnitude >= -normal_magnitude || visibility <= 0) &&
            (magnitude <= normal_magnitude || visibility >= 0)) {
            continue;
        }

        flag = 1;
        break;
    }

    return !flag;
}
#endif

void scene_allocate_textures(Scene *scene, int count, int length_64,
                             int length_128) {
    scene->texture_count = count;
    scene->texture_colours = calloc(count, sizeof(int8_t *));
    scene->texture_palette = calloc(count, sizeof(int32_t *));
    scene->texture_loaded_number = calloc(count, sizeof(int64_t));
    scene->texture_dimension = calloc(count, sizeof(int8_t));
    scene->texture_back_transparent = calloc(count, sizeof(int8_t));
    scene->texture_pixels = calloc(count, sizeof(int32_t *));

    scene_texture_count_loaded = 0;

    for (int i = 0; i < count; i++) {
        scene->texture_loaded_number[i] = 0;
    }

    /* 64x64 rgba */
    scene->texture_colours_64 = calloc(length_64, sizeof(int32_t *));
    scene->length_64 = length_64;

    /* 128x128 rgba */
    scene->texture_colours_128 = calloc(length_128, sizeof(int32_t *));
    scene->length_128 = length_128;
}

void scene_define_texture(Scene *scene, int id, int8_t *colours,
                          int32_t *palette, int is_128) {
    scene->texture_colours[id] = colours;
    scene->texture_palette[id] = palette;
    scene->texture_dimension[id] = is_128;

    scene->texture_loaded_number[id] = 0;
    scene->texture_back_transparent[id] = 0;
    scene->texture_pixels[id] = NULL;

    scene_prepare_texture(scene, id);
}

static void scene_prepare_texture(Scene *scene, int id) {
    if (id < 0) {
        return;
    }

    int max_id = 1 << 30;
    int old_id = 0;

    // TODO we can probably move this increment below the NULL check block
    scene_texture_count_loaded++;
    scene->texture_loaded_number[id] = scene_texture_count_loaded;

    /* without this we eventually get undefined behaviour */
    if (scene_texture_count_loaded == max_id - 1) {
        scene_texture_count_loaded = 0;
        scene->texture_loaded_number[id] = scene_texture_count_loaded;
    }

    if (scene->texture_pixels[id] != NULL) {
        return;
    }

    if (scene->texture_dimension[id] == 0) {
        // is 64 pixels wide
        for (int i = 0; i < scene->length_64; i++) {
            if (scene->texture_colours_64[i] == NULL) {
                scene->texture_colours_64[i] =
                    calloc(128 * 128, sizeof(int32_t));

                scene->texture_pixels[id] = scene->texture_colours_64[i];
                scene_set_texture_pixels(scene, id);
                return;
            }
        }
    } else {
        // is 128 wide
        for (int i = 0; i < scene->length_128; i++) {
            if (scene->texture_colours_128[i] == NULL) {
                scene->texture_colours_128[i] =
                    calloc(256 * 256, sizeof(int32_t));

                scene->texture_pixels[id] = scene->texture_colours_128[i];
                scene_set_texture_pixels(scene, id);
                return;
            }
        }
    }

    for (int i = 0; i < scene->texture_count; i++) {
        if (i != id &&
            scene->texture_dimension[i] == scene->texture_dimension[id] &&
            scene->texture_pixels[i] != NULL &&
            scene->texture_loaded_number[i] < max_id) {
            max_id = scene->texture_loaded_number[i];
            old_id = i;
        }
    }

    free(scene->texture_pixels[id]);

    scene->texture_pixels[id] = scene->texture_pixels[old_id];
    scene->texture_pixels[old_id] = NULL;

    scene_set_texture_pixels(scene, id);
}

static void scene_set_texture_pixels(Scene *scene, int id) {
    int texture_width = scene->texture_dimension[id] ? 128 : 64;
    int32_t *colours = scene->texture_pixels[id];
    int colour_count = 0;

    for (int x = 0; x < texture_width; x++) {
        for (int y = 0; y < texture_width; y++) {
            int colour =
                scene->texture_palette
                    [id]
                    [scene->texture_colours[id][y + x * texture_width] & 0xff];

            colour &= 0xf8f8ff;

            if (colour == 0) {
                colour = 1;
            } else if (colour == 0xf800ff) {
                colour = 0;
                scene->texture_back_transparent[id] = 1;
            }

            colours[colour_count++] = colour;
        }
    }

    for (int i = 0; i < colour_count; i++) {
        int32_t colour = colours[i];

        colours[colour_count + i] = (colour - (colour >> 3)) & 0xf8f8ff;
        colours[colour_count * 2 + i] = (colour - (colour >> 2)) & 0xf8f8ff;

        colours[colour_count * 3 + i] =
            (colour - (colour >> 2) - (colour >> 3)) & 0xf8f8ff;
    }
}

#ifdef RENDER_SW
void scene_scroll_texture(Scene *scene, int id) {
    if (scene->texture_pixels[id] == NULL) {
        return;
    }

    int32_t *colours = scene->texture_pixels[id];
    int S = SCROLL_TEXTURE_SIZE;
    int area = SCROLL_TEXTURE_AREA; // = S * S
    int32_t tmp_row[/* max S */ 512];
    memcpy(tmp_row, &colours[area - S], S * sizeof(int32_t));
    memmove(&colours[S],
            &colours[0],
            (area - S) * sizeof(int32_t));

    memcpy(&colours[0],
           tmp_row,
           S * sizeof(int32_t));

    int32_t *base_layer = colours;
    int32_t *layer1 = colours + area;
    int32_t *layer2 = colours + 2 * area;
    int32_t *layer3 = colours + 3 * area;

    for (int i = 0; i < area; i++) {
        int32_t c = base_layer[i];

        int32_t c_sub1 = c - (c >> 3);         // used for layer1
        int32_t c_sub2 = c - (c >> 2);         // used for layer2
        int32_t c_sub3 = c_sub2 - (c >> 3);    // c - (c>>2) - (c>>3)

        c_sub1 &= 0xf8f8ff;
        c_sub2 &= 0xf8f8ff;
        c_sub3 &= 0xf8f8ff;

        layer1[i] = c_sub1;
        layer2[i] = c_sub2;
        layer3[i] = c_sub3;
    }
}
#endif

int16_t scene_rgb_to_fill(uint8_t r, uint8_t g, uint8_t b) {
    return -1 - ((r >> 3) << 10) - ((g >> 3) << 5) - (b >> 3);
}

/* used to convert face_fill values (textures or colours) to minimap colours */
int scene_get_fill_colour(Scene *scene, int face_fill) {
#ifdef USE_TOONSCAPE
    face_fill = apply_toonscape(face_fill);
#endif

    if (face_fill == COLOUR_TRANSPARENT) {
        return 0;
    }

#ifdef RENDER_SW
    scene_prepare_texture(scene, face_fill);

    if (face_fill >= 0) {
        return scene->texture_pixels[face_fill][0];
    }
#elif defined(RENDER_GL)
#ifdef DREAMCAST
    return 0xFFFFFF; // default white or whatever
#else
    if (face_fill >= 0) {
        gl_atlas_position atlas_position =
            gl_texture_atlas_positions[face_fill];

        int x = (int)(atlas_position.left_u * GL_TEXTURE_SIZE);
        int y = (int)(atlas_position.top_v * GL_TEXTURE_SIZE);

        uint32_t *texture_pixels = (uint32_t *)scene->gl_model_surface->pixels;
        uint32_t pixel = texture_pixels[(y * 1024) + x];

        uint8_t r, g, b;
        SDL_GetRGB(pixel, scene->gl_model_surface->format, &r, &g, &b);

        return (r << 16 | g << 8 | b);
    }
#endif

    if (face_fill >= 0) {
        gl_atlas_position atlas_position =
            gl_texture_atlas_positions[face_fill];

        int x = (int)(atlas_position.left_u * GL_TEXTURE_SIZE);
        int y = (int)(atlas_position.top_v * GL_TEXTURE_SIZE);

        uint32_t *texture_pixels = (uint32_t *)scene->gl_model_surface->pixels;
        uint32_t pixel = texture_pixels[(y * 1024) + x];

        uint8_t r, g, b;
        SDL_GetRGB(pixel, scene->gl_model_surface->format, &r, &g, &b);

        return (r << 16 | g << 8 | b);
    }
#elif defined(RENDER_3DS_GL)
    if (face_fill >= 0) {
        gl_atlas_position atlas_position =
            gl_texture_atlas_positions[face_fill];

        int x = (int)(atlas_position.left_u * GL_TEXTURE_SIZE);
        int y = (int)(atlas_position.top_v * GL_TEXTURE_SIZE);

        int offset = _3ds_gl_translate_texture_index(x, y, 1024) / 2;
        uint16_t *data = (uint16_t *)scene->gl_model_texture.data;
        uint16_t colour16 = data[offset];

        return _3ds_gl_rgba5551_to_rgb32(colour16);
    }
#endif

    if (face_fill < 0) {
        face_fill = -(face_fill + 1);

        int r = (face_fill >> 10) & 0x1f;
        int g = (face_fill >> 5) & 0x1f;
        int b = face_fill & 0x1f;

        return (r << 19) + (g << 11) + (b << 3);
    }

    return 0;
}

void scene_set_light_dir(Scene *scene, int x, int y, int z) {
    if (x == 0 && y == 0 && z == 0) {
        x = 32;
    }

    for (int i = 0; i < scene->model_count; i++) {
        game_model_set_light_dir(scene->models[i], x, y, z);
    }
}

void scene_set_light(Scene *scene, int ambience, int diffuse, int x, int y,
                     int z) {
    if (x == 0 && y == 0 && z == 0) {
        x = 32;
    }

    for (int i = 0; i < scene->model_count; i++) {
        game_model_set_light_intensity(scene->models[i], ambience, diffuse, x,
                                       y, z);
    }
}

#ifdef RENDER_SW
static int scene_method306(int i, int j, int k, int l, int i1) {
    if (l == j) {
        return i;
    }

    return i + (((k - i) * (i1 - j)) / (l - j));
}

static int scene_method307(int i, int j, int k, int l, int flag) {
    if ((flag && i <= k) || i < k) {
        if (i > l) {
            return 1;
        }

        if (j > k) {
            return 1;
        }

        if (j > l) {
            return 1;
        }

        return !flag;
    }

    if (i < l) {
        return 1;
    }

    if (j < k) {
        return 1;
    }

    if (j < l) {
        return 1;
    }

    return flag;
}

static int scene_method308(int i, int j, int k, int flag) {
    if ((flag && i <= k) || i < k) {
        if (j > k) {
            return 1;
        }

        return !flag;
    }

    if (j < k) {
        return 1;
    }

    return flag;
}

static inline int wrap_a(int i, int len) {
    // Ensures an index stays within [0..len-1]
    return (i + len) % len;
}

static inline int wrap_b(int i, int len) {
    // Same for polygon B
    return (i + len) % len;
}

static int scene_intersect(int *vertex_view_x_a, int *vertex_view_y_a,
                           int *vertex_view_x_b, int *vertex_view_y_b,
                           int length_a, int length_b) {

    int *xA = vertex_view_x_a;
    int *yA = vertex_view_y_a;
    int *xB = vertex_view_x_b;
    int *yB = vertex_view_y_b;

    int view_y_a = yA[0];
    int k20 = view_y_a;
    int k = 0;
    for (int i = 1; i < length_a; i++) {
        int yVal = yA[i];
        if (yVal < view_y_a) {
            view_y_a = yVal;
            k = i;
        } else if (yVal > k20) {
            k20 = yVal;
        }
    }

    int view_y_b = yB[0];
    int l20 = view_y_b;
    int i1 = 0;
    for (int i = 1; i < length_b; i++) {
        int yVal = yB[i];
        if (yVal < view_y_b) {
            view_y_b = yVal;
            i1 = i;
        } else if (yVal > l20) {
            l20 = yVal;
        }
    }

    if (view_y_b >= k20 || view_y_a >= l20) {
        return 0;
    }

    int l = 0;
    int j1 = 0;
    int flag = 0;
    int8_t byte0 = 0;

    if (yA[k] < yB[i1]) {
        for (l = k; yA[l] < yB[i1]; l = wrap_a(l + 1, length_a))
            ;
        for (; yA[k] < yB[i1]; k = wrap_a(k - 1, length_a))
            ;

        int k1 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                 yA[wrap_a(k + 1, length_a)],
                                 xA[k], yA[k],
                                 yB[i1]);

        int k6 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                 yA[wrap_a(l - 1, length_a)],
                                 xA[l], yA[l],
                                 yB[i1]);

        int l10 = xB[i1];
        flag = (k1 < l10) | (k6 < l10);

        if (scene_method308(k1, k6, l10, flag)) {
            return 1;
        }

        j1 = wrap_b(i1 + 1, length_b);
        i1 = wrap_b(i1 - 1, length_b);

        if (k == l) {
            byte0 = 1;
        }
    } else {
        for (j1 = i1; yB[j1] < yA[k]; j1 = wrap_b(j1 + 1, length_b))
            ;
        for (; yB[i1] < yA[k]; i1 = wrap_b(i1 - 1, length_b))
            ;

        int l1 = xA[k];

        int i11 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                  yB[wrap_b(i1 + 1, length_b)],
                                  xB[i1], yB[i1],
                                  yA[k]);

        int l15 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                  yB[wrap_b(j1 - 1, length_b)],
                                  xB[j1], yB[j1],
                                  yA[k]);

        flag = (l1 < i11) | (l1 < l15);

        if (scene_method308(i11, l15, l1, !flag)) {
            return 1;
        }

        l = wrap_a(k + 1, length_a);
        k = wrap_a(k - 1, length_a);

        if (i1 == j1) {
            byte0 = 2;
        }
    }

    while (byte0 == 0) {
        if (yA[k] < yA[l]) {
            if (yA[k] < yB[i1]) {
                if (yA[k] < yB[j1]) {
                    int i2 = xA[k];

                    int l6 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                             yA[wrap_a(l - 1, length_a)],
                                             xA[l], yA[l], yA[k]);

                    int j11 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                              yB[wrap_b(i1 + 1, length_b)],
                                              xB[i1], yB[i1], yA[k]);

                    int i16 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                              yB[wrap_b(j1 - 1, length_b)],
                                              xB[j1], yB[j1], yA[k]);

                    if (scene_method307(i2, l6, j11, i16, flag)) {
                        return 1;
                    }
                    k = wrap_a(k - 1, length_a);

                    if (k == l) {
                        byte0 = 1;
                    }
                } else {
                    int j2 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                             yA[wrap_a(k + 1, length_a)],
                                             xA[k], yA[k], yB[j1]);

                    int i7 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                             yA[wrap_a(l - 1, length_a)],
                                             xA[l], yA[l], yB[j1]);

                    int k11 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                              yB[wrap_b(i1 + 1, length_b)],
                                              xB[i1], yB[i1], yB[j1]);

                    int j16 = xB[j1];

                    if (scene_method307(j2, i7, k11, j16, flag)) {
                        return 1;
                    }
                    j1 = wrap_b(j1 + 1, length_b);
                    if (i1 == j1) {
                        byte0 = 2;
                    }
                }
            } else if (yB[i1] < yB[j1]) {
                int k2 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                         yA[wrap_a(k + 1, length_a)],
                                         xA[k], yA[k], yB[i1]);

                int j7 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                         yA[wrap_a(l - 1, length_a)],
                                         xA[l], yA[l], yB[i1]);

                int l11 = xB[i1];

                int k16 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                          yB[wrap_b(j1 - 1, length_b)],
                                          xB[j1], yB[j1], yB[i1]);

                if (scene_method307(k2, j7, l11, k16, flag)) {
                    return 1;
                }
                i1 = wrap_b(i1 - 1, length_b);
                if (i1 == j1) {
                    byte0 = 2;
                }
            } else {
                int l2 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                         yA[wrap_a(k + 1, length_a)],
                                         xA[k], yA[k], yB[j1]);

                int k7 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                         yA[wrap_a(l - 1, length_a)],
                                         xA[l], yA[l], yB[j1]);

                int i12 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                          yB[wrap_b(i1 + 1, length_b)],
                                          xB[i1], yB[i1], yB[j1]);

                int l16 = xB[j1];

                if (scene_method307(l2, k7, i12, l16, flag)) {
                    return 1;
                }
                j1 = wrap_b(j1 + 1, length_b);
                if (i1 == j1) {
                    byte0 = 2;
                }
            }
        } else if (yA[l] < yB[i1]) {
            if (yA[l] < yB[j1]) {
                int i3 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                         yA[wrap_a(k + 1, length_a)],
                                         xA[k], yA[k], yA[l]);

                int l7 = xA[l];

                int j12 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                          yB[wrap_b(i1 + 1, length_b)],
                                          xB[i1], yB[i1], yA[l]);

                int i17 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                          yB[wrap_b(j1 - 1, length_b)],
                                          xB[j1], yB[j1], yA[l]);

                if (scene_method307(i3, l7, j12, i17, flag)) {
                    return 1;
                }
                l = wrap_a(l + 1, length_a);
                if (k == l) {
                    byte0 = 1;
                }
            } else {
                int j3 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                         yA[wrap_a(k + 1, length_a)],
                                         xA[k], yA[k], yB[j1]);

                int i8 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                         yA[wrap_a(l - 1, length_a)],
                                         xA[l], yA[l], yB[j1]);

                int k12 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                          yB[wrap_b(i1 + 1, length_b)],
                                          xB[i1], yB[i1], yB[j1]);

                int j17 = xB[j1];

                if (scene_method307(j3, i8, k12, j17, flag)) {
                    return 1;
                }
                j1 = wrap_b(j1 + 1, length_b);
                if (i1 == j1) {
                    byte0 = 2;
                }
            }
        } else if (yB[i1] < yB[j1]) {
            int k3 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yB[i1]);

            int j8 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                     yA[wrap_a(l - 1, length_a)],
                                     xA[l], yA[l], yB[i1]);

            int l12 = xB[i1];

            int k17 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                      yB[wrap_b(j1 - 1, length_b)],
                                      xB[j1], yB[j1], yB[i1]);

            if (scene_method307(k3, j8, l12, k17, flag)) {
                return 1;
            }
            i1 = wrap_b(i1 - 1, length_b);
            if (i1 == j1) {
                byte0 = 2;
            }
        } else {
            int l3 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yB[j1]);

            int k8 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                     yA[wrap_a(l - 1, length_a)],
                                     xA[l], yA[l], yB[j1]);

            int i13 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                      yB[wrap_b(i1 + 1, length_b)],
                                      xB[i1], yB[i1], yB[j1]);

            int l17 = xB[j1];

            if (scene_method307(l3, k8, i13, l17, flag)) {
                return 1;
            }
            j1 = wrap_b(j1 + 1, length_b);
            if (i1 == j1) {
                byte0 = 2;
            }
        }
    }

    while (byte0 == 1) {
        if (yA[k] < yB[i1]) {
            if (yA[k] < yB[j1]) {
                int i4 = xA[k];

                int j13 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                          yB[wrap_b(i1 + 1, length_b)],
                                          xB[i1], yB[i1], yA[k]);

                int i18 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                          yB[wrap_b(j1 - 1, length_b)],
                                          xB[j1], yB[j1], yA[k]);

                return scene_method308(j13, i18, i4, !flag);
            }

            int j4 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yB[j1]);

            int l8 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                     yA[wrap_a(l - 1, length_a)],
                                     xA[l], yA[l], yB[j1]);

            int k13 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                      yB[wrap_b(i1 + 1, length_b)],
                                      xB[i1], yB[i1], yB[j1]);

            int j18 = xB[j1];

            if (scene_method307(j4, l8, k13, j18, flag)) {
                return 1;
            }
            j1 = wrap_b(j1 + 1, length_b);
            if (i1 == j1) {
                byte0 = 0;
            }
        } else if (yB[i1] < yB[j1]) {
            int k4 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yB[i1]);

            int i9 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                     yA[wrap_a(l - 1, length_a)],
                                     xA[l], yA[l], yB[i1]);

            int l13 = xB[i1];

            int k18 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                      yB[wrap_b(j1 - 1, length_b)],
                                      xB[j1], yB[j1], yB[i1]);

            if (scene_method307(k4, i9, l13, k18, flag)) {
                return 1;
            }
            i1 = wrap_b(i1 - 1, length_b);
            if (i1 == j1) {
                byte0 = 0;
            }
        } else {
            int l4 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yB[j1]);

            int j9 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                     yA[wrap_a(l - 1, length_a)],
                                     xA[l], yA[l], yB[j1]);

            int i14 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                      yB[wrap_b(i1 + 1, length_b)],
                                      xB[i1], yB[i1], yB[j1]);

            int l18 = xB[j1];

            if (scene_method307(l4, j9, i14, l18, flag)) {
                return 1;
            }
            j1 = wrap_b(j1 + 1, length_b);
            if (i1 == j1) {
                byte0 = 0;
            }
        }
    }

    while (byte0 == 2) {
        if (yB[i1] < yA[k]) {
            if (yB[i1] < yA[l]) {
                int i5 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                         yA[wrap_a(k + 1, length_a)],
                                         xA[k], yA[k], yB[i1]);

                int k9 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                         yA[wrap_a(l - 1, length_a)],
                                         xA[l], yA[l], yB[i1]);

                int j14 = xB[i1];

                return scene_method308(i5, k9, j14, flag);
            }

            int j5 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yA[l]);

            int l9 = xA[l];

            int k14 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                      yB[wrap_b(i1 + 1, length_b)],
                                      xB[i1], yB[i1], yA[l]);

            int i19 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                      yB[wrap_b(j1 - 1, length_b)],
                                      xB[j1], yB[j1], yA[l]);

            if (scene_method307(j5, l9, k14, i19, flag)) {
                return 1;
            }
            l = wrap_a(l + 1, length_a);
            if (k == l) {
                byte0 = 0;
            }
        } else if (yA[k] < yA[l]) {
            int k5 = xA[k];

            int i10 = scene_method306(xA[wrap_a(l - 1, length_a)],
                                      yA[wrap_a(l - 1, length_a)],
                                      xA[l], yA[l], yA[k]);

            int l14 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                      yB[wrap_b(i1 + 1, length_b)],
                                      xB[i1], yB[i1], yA[k]);

            int j19 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                      yB[wrap_b(j1 - 1, length_b)],
                                      xB[j1], yB[j1], yA[k]);

            if (scene_method307(k5, i10, l14, j19, flag)) {
                return 1;
            }
            k = wrap_a(k - 1, length_a);
            if (k == l) {
                byte0 = 0;
            }
        } else {
            int l5 = scene_method306(xA[wrap_a(k + 1, length_a)],
                                     yA[wrap_a(k + 1, length_a)],
                                     xA[k], yA[k], yA[l]);

            int j10 = xA[l];

            int i15 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                      yB[wrap_b(i1 + 1, length_b)],
                                      xB[i1], yB[i1], yA[l]);

            int k19 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                      yB[wrap_b(j1 - 1, length_b)],
                                      xB[j1], yB[j1], yA[l]);

            if (scene_method307(l5, j10, i15, k19, flag)) {
                return 1;
            }
            l = wrap_a(l + 1, length_a);
            if (k == l) {
                byte0 = 0;
            }
        }
    }

    if (yA[k] < yB[i1]) {
        int i6 = xA[k];

        int j15 = scene_method306(xB[wrap_b(i1 + 1, length_b)],
                                  yB[wrap_b(i1 + 1, length_b)],
                                  xB[i1], yB[i1], yA[k]);

        int l19 = scene_method306(xB[wrap_b(j1 - 1, length_b)],
                                  yB[wrap_b(j1 - 1, length_b)],
                                  xB[j1], yB[j1], yA[k]);

        return scene_method308(j15, l19, i6, !flag);
    }

    int j6 = scene_method306(xA[wrap_a(k + 1, length_a)],
                             yA[wrap_a(k + 1, length_a)],
                             xA[k], yA[k],
                             yB[i1]);

    int k10 = scene_method306(xA[wrap_a(l - 1, length_a)],
                              yA[wrap_a(l - 1, length_a)],
                              xA[l], yA[l],
                              yB[i1]);

    int k15 = xB[i1];

    return scene_method308(j6, k10, k15, flag);
}
#endif /* RENDER_SW */

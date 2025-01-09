#include "mudclient.h"
#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/keyboard.h>
#include <dc/maple/mouse.h>
#include <dc/maple/controller.h>
#include <dc/pvr.h>
#include <dc/video.h>
#include <dc/sound/stream.h>
#include <SDL/SDL.h>
#include <ctype.h>
#include <kos/dbglog.h>

// Debug text color
#define DEBUG_TEXT_COLOR PVR_PACK_COLOR(1.0, 1.0, 1.0, 1.0)

// Framebuffer dimensions
#define FRAMEBUFFER_WIDTH 640
#define FRAMEBUFFER_HEIGHT 480

//Keyboard Buffer
#define DREAMCAST_KEYBOARD_BUFFER_SIZE 256

// Adjusted sensitivity scaling for 640x480 resolution
#define MOUSE_SENSITIVITY 0.0001f  // Adjust sensitivity here
#define SCALE_X ((float)RENDER_WIDTH / FRAMEBUFFER_WIDTH)
#define SCALE_Y ((float)RENDER_HEIGHT / FRAMEBUFFER_HEIGHT)

// D-Pad movement speed (pixels per frame)
#define DPAD_SPEED 5

// Keyboard mappings for (TODO) Maple Keyboard Handling
#ifdef DREAMCAST
char keyboard_buttons[5][10] = {
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'},
    {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'},
    {' ', ' ', ' ', ' ', ' ', ' ', '-', '=', '\'', 0}};

char keyboard_shift_buttons[5][10] = {
    {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'},
    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':'},
    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'},
    {' ', ' ', ' ', ' ', ' ', ' ', '_', '+', '"', 0}};
#endif

void append_to_keyboard_buffer(char c);
void process_backspace(void);

void draw_cursor(int cursor_x, int cursor_y) {
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr; // Declare hdr here

    // Configure drawing context
    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    // Draw a simple crosshair or dot
    pvr_vertex_t vert;

    // Top-left corner
    vert.flags = PVR_CMD_VERTEX;
    vert.x = cursor_x - 5;
    vert.y = cursor_y - 5;
    vert.z = 1.0f;
    vert.u = vert.v = 0.0f;
    vert.argb = PVR_PACK_COLOR(1.0, 1.0, 1.0, 1.0);
    vert.oargb = 0;
    pvr_prim(&vert, sizeof(vert));

    // Top-right corner
    vert.x = cursor_x + 5;
    vert.y = cursor_y - 5;
    pvr_prim(&vert, sizeof(vert));

    // Bottom-right corner
    vert.x = cursor_x + 5;
    vert.y = cursor_y + 5;
    pvr_prim(&vert, sizeof(vert));

    // Bottom-left corner
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = cursor_x - 5;
    vert.y = cursor_y + 5;
    pvr_prim(&vert, sizeof(vert));
}

void mudclient_poll_events(mudclient *mud) {
    static int cursor_x = FRAMEBUFFER_WIDTH / 2;
    static int cursor_y = FRAMEBUFFER_HEIGHT / 2;

    // Variables for smoothing and delta management
    static float avg_dx = 0.0f, avg_dy = 0.0f;
    static int last_dx = 0, last_dy = 0;
    static uint32_t last_mouse_update = 0;

#ifdef DREAMCAST
    // Handle Maple Mouse
    maple_device_t *mouse = maple_enum_type(0, MAPLE_FUNC_MOUSE);
    if (mouse) {
        mouse_state_t *mstate = (mouse_state_t *)maple_dev_status(mouse);
        if (mstate) {
            uint32_t current_time = timer_ms_gettime64();
            uint32_t delta_time = current_time - last_mouse_update;
            last_mouse_update = current_time;

            // Apply exponential moving average for smoothing
            const float alpha = 0.2f; // Smoothing factor (adjustable for better responsiveness)
            avg_dx = alpha * mstate->dx + (1.0f - alpha) * avg_dx;
            avg_dy = alpha * mstate->dy + (1.0f - alpha) * avg_dy;

            // Apply fine-grained sensitivity
            float dynamic_sensitivity = 0.000005f; // Minimal sensitivity for crawling movement
            int smooth_dx = (int)(avg_dx * dynamic_sensitivity);
            int smooth_dy = (int)(avg_dy * dynamic_sensitivity);

            // Clamp deltas to avoid erratic movement
            smooth_dx = (smooth_dx > 2) ? 2 : (smooth_dx < -2 ? -2 : smooth_dx);
            smooth_dy = (smooth_dy > 2) ? 2 : (smooth_dy < -2 ? -2 : smooth_dy);

            // Update cursor position
            cursor_x += smooth_dx;
            cursor_y += smooth_dy;

            // Ensure cursor stays within screen bounds
            cursor_x = (cursor_x < 0) ? 0 : (cursor_x >= FRAMEBUFFER_WIDTH ? FRAMEBUFFER_WIDTH - 1 : cursor_x);
            cursor_y = (cursor_y < 0) ? 0 : (cursor_y >= FRAMEBUFFER_HEIGHT ? FRAMEBUFFER_HEIGHT - 1 : cursor_y);

            // Handle mouse buttons
            if (mstate->buttons & MOUSE_LEFTBUTTON) {
                mudclient_mouse_pressed(mud, cursor_x, cursor_y, 1);
            } else {
                mudclient_mouse_released(mud, cursor_x, cursor_y, 1);
            }

            if (mstate->buttons & MOUSE_RIGHTBUTTON) {
                mudclient_mouse_pressed(mud, cursor_x, cursor_y, 3);
            } else {
                mudclient_mouse_released(mud, cursor_x, cursor_y, 3);
            }

            if (mstate->buttons & MOUSE_MIDDLEBUTTON) {
                mudclient_mouse_pressed(mud, cursor_x, cursor_y, 2);
            } else {
                mudclient_mouse_released(mud, cursor_x, cursor_y, 2);
            }

            mudclient_mouse_moved(mud, cursor_x, cursor_y);
        }
    }

    // Handle Dreamcast Controller
    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (cont) {
        cont_state_t *cstate = (cont_state_t *)maple_dev_status(cont);
        if (cstate) {
            if (cstate->buttons & CONT_DPAD_UP) cursor_y -= DPAD_SPEED;
            if (cstate->buttons & CONT_DPAD_DOWN) cursor_y += DPAD_SPEED;
            if (cstate->buttons & CONT_DPAD_LEFT) cursor_x -= DPAD_SPEED;
            if (cstate->buttons & CONT_DPAD_RIGHT) cursor_x += DPAD_SPEED;

            // Handle A and B buttons for mouse clicks
            if (cstate->buttons & CONT_A) {
                mudclient_mouse_pressed(mud, cursor_x, cursor_y, 1);
            } else {
                mudclient_mouse_released(mud, cursor_x, cursor_y, 1);
            }

            if (cstate->buttons & CONT_B) {
                mudclient_mouse_pressed(mud, cursor_x, cursor_y, 3);
            } else {
                mudclient_mouse_released(mud, cursor_x, cursor_y, 3);
            }

            mudclient_mouse_moved(mud, cursor_x, cursor_y);
        }
    }
#endif

    // Draw the cursor persistently
    draw_cursor(cursor_x, cursor_y);

    // SDL Keyboard Handling
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN: {
                char char_code = -1;
                int code = -1;
                get_sdl_keycodes(&event.key.keysym, &char_code, &code);
                if (code != -1) {
                    dbglog(DBG_INFO, "Key pressed: %d (%c)\n", code, char_code);
                    if (char_code != -1 && isprint(char_code)) {
                        append_to_keyboard_buffer(char_code);
                    } else if (code == SDLK_BACKSPACE) {
                        process_backspace();
                    }
                    mudclient_key_pressed(mud, code, char_code);
                }
            } break;

            case SDL_KEYUP: {
                char char_code = -1;
                int code = -1;
                get_sdl_keycodes(&event.key.keysym, &char_code, &code);
                if (code != -1) {
                    dbglog(DBG_INFO, "Key released: %d\n", code);
                    mudclient_key_released(mud, code);
                }
            } break;

            default:
                break;
        }
    }
}

char dreamcast_keyboard_buffer[DREAMCAST_KEYBOARD_BUFFER_SIZE];
int keyboard_buffer_pos = 0;

void reset_keyboard_buffer() {
    memset(dreamcast_keyboard_buffer, 0, DREAMCAST_KEYBOARD_BUFFER_SIZE);
    keyboard_buffer_pos = 0;
}

void append_to_keyboard_buffer(char c) {
    if (keyboard_buffer_pos < DREAMCAST_KEYBOARD_BUFFER_SIZE - 1) {
        dreamcast_keyboard_buffer[keyboard_buffer_pos++] = c;
        dreamcast_keyboard_buffer[keyboard_buffer_pos] = '\0';
    } else {
        dbglog(DBG_WARNING, "Keyboard buffer full, cannot append character: %c\n", c);
    }
}

void process_backspace() {
    if (keyboard_buffer_pos > 0) {
        dreamcast_keyboard_buffer[--keyboard_buffer_pos] = '\0';
    }
}

void update_cursor_position(int *cursor_x, int *cursor_y, float delta_x, float delta_y) {
    float sensitivity = 0.000005f; // Minimal sensitivity for crawling movement

    // Apply sensitivity and cap deltas
    int capped_dx = (int)(delta_x * sensitivity);
    int capped_dy = (int)(delta_y * sensitivity);

    capped_dx = (capped_dx > 2) ? 2 : (capped_dx < -2 ? -2 : capped_dx);
    capped_dy = (capped_dy > 2) ? 2 : (capped_dy < -2 ? -2 : capped_dy);

    *cursor_x += capped_dx;
    *cursor_y += capped_dy;

    // Clamp cursor position to screen bounds
    *cursor_x = (*cursor_x < 0) ? 0 : (*cursor_x >= FRAMEBUFFER_WIDTH ? FRAMEBUFFER_WIDTH - 1 : *cursor_x);
    *cursor_y = (*cursor_y < 0) ? 0 : (*cursor_y >= FRAMEBUFFER_HEIGHT ? FRAMEBUFFER_HEIGHT - 1 : *cursor_y);
}

void mudclient_start_application(mudclient *mud, char *title) {
#ifdef DREAMCAST
    vid_init(DM_640x480, PM_RGB565);
    pvr_init_defaults();
    pvr_set_bg_color(0.0f, 0.0f, 0.0f);
    snd_stream_init();
    maple_init();
    dbglog(DBG_INFO, "Mudclient initialized for Dreamcast\n");
#else
    int init = SDL_INIT_VIDEO | SDL_INIT_TIMER;
    if (SDL_Init(init) < 0) {
        mud_error("SDL_Init(): %s\n", SDL_GetError());
        exit(1);
    }

    SDL_EnableUNICODE(1);
    SDL_WM_SetCaption(title, NULL);

#ifdef RENDER_SW
    mud->screen = SDL_SetVideoMode(mud->game_width, mud->game_height, 32,
                                   SDL_HWSURFACE | SDL_RESIZABLE);
#elif defined(RENDER_GL)
    mud->screen = SDL_SetVideoMode(mud->game_width, mud->game_height, 32,
                                   SDL_OPENGL | SDL_RESIZABLE);
#endif
#endif
}

#ifdef SDL12
void get_sdl_keycodes(SDL_keysym *keysym, char *char_code, int *code) {
    *code = -1;
    *char_code = -1;

    if (keysym->unicode > 0 && keysym->unicode < 128) {
        if (isprint((unsigned char)keysym->unicode)) {
            *code = keysym->unicode;
            *char_code = keysym->unicode;
            return;
        }
    }

    switch (keysym->sym) {
        case SDLK_TAB: *code = K_TAB; *char_code = '\t'; break;
        case SDLK_BACKSPACE: *code = K_BACKSPACE; *char_code = '\b'; break;
        case SDLK_LEFT: *code = K_LEFT; break;
        case SDLK_RIGHT: *code = K_RIGHT; break;
        case SDLK_UP: *code = K_UP; break;
        case SDLK_DOWN: *code = K_DOWN; break;
        case SDLK_PAGEUP: *code = K_PAGE_UP; break;
        case SDLK_PAGEDOWN: *code = K_PAGE_DOWN; break;
        case SDLK_HOME: *code = K_HOME; break;
        case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5:
        case SDLK_F6: case SDLK_F7: case SDLK_F8: case SDLK_F9:
        case SDLK_F10: case SDLK_F11: case SDLK_F12:
            *code = keysym->sym; break; // Function keys
        case SDLK_ESCAPE: *code = K_ESCAPE; break;
        case SDLK_RETURN: *code = K_ENTER; *char_code = '\r'; break;
        case SDLK_SPACE: *code = ' '; *char_code = ' '; break;
        case SDLK_COMMA: *code = ','; *char_code = ','; break;
        case SDLK_PERIOD: *code = '.'; *char_code = '.'; break;
        case SDLK_QUOTE: *code = '\''; *char_code = '\''; break;
        case SDLK_BACKQUOTE: *code = '`'; *char_code = '`'; break;
        case SDLK_CAPSLOCK: *code = KMOD_CAPS; break;
        case SDLK_LSHIFT: case SDLK_RSHIFT: *code = KMOD_SHIFT; break;

        case SDLK_a: case SDLK_b: case SDLK_c: case SDLK_d: case SDLK_e:
        case SDLK_f: case SDLK_g: case SDLK_h: case SDLK_i: case SDLK_j:
        case SDLK_k: case SDLK_l: case SDLK_m: case SDLK_n: case SDLK_o:
        case SDLK_p: case SDLK_q: case SDLK_r: case SDLK_s: case SDLK_t:
        case SDLK_u: case SDLK_v: case SDLK_w: case SDLK_x: case SDLK_y:
        case SDLK_z:
            *code = keysym->sym;
            *char_code = keysym->sym; // Handles lowercase letters
            break;

        case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
        case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
            *code = keysym->sym;
            *char_code = keysym->sym; // Numeric characters
            break;

        // Numpad Keys
        case SDLK_KP0: case SDLK_KP1: case SDLK_KP2: case SDLK_KP3:
        case SDLK_KP4: case SDLK_KP5: case SDLK_KP6: case SDLK_KP7:
        case SDLK_KP8: case SDLK_KP9: case SDLK_KP_PERIOD:
        case SDLK_KP_DIVIDE: case SDLK_KP_MULTIPLY: case SDLK_KP_MINUS:
        case SDLK_KP_PLUS: case SDLK_KP_ENTER: case SDLK_KP_EQUALS:
            *code = keysym->sym;
            *char_code = keysym->unicode; // Handles numpad characters
            break;

        default:
            if (keysym->unicode > 0) {
                *code = keysym->unicode;
                *char_code = keysym->unicode;
            }
            break;
    }
}
#endif

#include "mudclient.h"
#include <kos.h>
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

// Keyboard Buffer
#define DREAMCAST_KEYBOARD_BUFFER_SIZE 256

// Adjusted sensitivity scaling for 640x480 resolution
#define SCALE_X ((float)RENDER_WIDTH / FRAMEBUFFER_WIDTH)
#define SCALE_Y ((float)RENDER_HEIGHT / FRAMEBUFFER_HEIGHT)

// D-Pad movement speed (pixels per frame)
#define DPAD_SPEED 5

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

// We keep these static variables for tracking mouse position
static int s_cursor_x = FRAMEBUFFER_WIDTH / 2;
static int s_cursor_y = FRAMEBUFFER_HEIGHT / 2;

// Forward declarations
void append_to_keyboard_buffer(char c);
void process_backspace(void);

/*
    Removed: draw_cursor(...) and mudclient_draw_cursor(...)
    The game’s own engine or higher-level code presumably handles the GUI cursor.
*/

// This function just polls events and updates s_cursor_x, s_cursor_y
void mudclient_poll_events(mudclient *mud) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

            case SDL_QUIT:
                exit(0);
                break;

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
                break;
            }

            case SDL_KEYUP: {
                char char_code = -1;
                int code = -1;
                get_sdl_keycodes(&event.key.keysym, &char_code, &code);
                if (code != -1) {
                    dbglog(DBG_INFO, "Key released: %d\n", code);
                    mudclient_key_released(mud, code);
                }
                break;
            }

            case SDL_MOUSEMOTION: {
                printf("xrel=%d, yrel=%d\n", event.motion.xrel, event.motion.yrel);
                s_cursor_x += event.motion.xrel;
                s_cursor_y += event.motion.yrel;

                // Update game with new position
                mudclient_mouse_moved(mud, s_cursor_x, s_cursor_y);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == 4) { // Scroll up
                    mud->mouse_scroll_delta = -1;
                } else if (event.button.button == 5) { // Scroll down
                    mud->mouse_scroll_delta = 1;
                } else {
                    mudclient_mouse_pressed(mud, event.button.x, event.button.y,
                                            event.button.button);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                mudclient_mouse_released(mud, event.button.x, event.button.y,
                                         event.button.button);
                break;

            case SDL_JOYAXISMOTION: {
                // Joystick movements also update the same cursor
                if (event.jaxis.axis == 0) { // Left Stick X-axis
                    s_cursor_x += (int)((event.jaxis.value / 32767.0f) * DPAD_SPEED);
                } else if (event.jaxis.axis == 1) { // Left Stick Y-axis
                    s_cursor_y += (int)((event.jaxis.value / 32767.0f) * DPAD_SPEED);
                }

                // Notify game
                mudclient_mouse_moved(mud, s_cursor_x, s_cursor_y);
                break;
            }

            case SDL_JOYBUTTONDOWN:
                // Use the internal cursor location for clicks
                if (event.jbutton.button == 0) { // A => left
                    mudclient_mouse_pressed(mud, s_cursor_x, s_cursor_y, 1);
                } else if (event.jbutton.button == 1) { // B => right
                    mudclient_mouse_pressed(mud, s_cursor_x, s_cursor_y, 3);
                }
                break;

            case SDL_JOYBUTTONUP:
                if (event.jbutton.button == 0) {
                    mudclient_mouse_released(mud, s_cursor_x, s_cursor_y, 1);
                } else if (event.jbutton.button == 1) {
                    mudclient_mouse_released(mud, s_cursor_x, s_cursor_y, 3);
                }
                break;

            default:
                break;
        } // end switch
    } // end while (SDL_PollEvent)

    // Final boundary check after all events
    if (s_cursor_x < 0) s_cursor_x = 0;
    if (s_cursor_x >= FRAMEBUFFER_WIDTH)  s_cursor_x = FRAMEBUFFER_WIDTH - 1;
    if (s_cursor_y < 0) s_cursor_y = 0;
    if (s_cursor_y >= FRAMEBUFFER_HEIGHT) s_cursor_y = FRAMEBUFFER_HEIGHT - 1;
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

void update_cursor_position(int *cursor_x, int *cursor_y, int delta_x, int delta_y) {
    // No sensitivity, no max delta - direct additions
    *cursor_x += delta_x;
    *cursor_y += delta_y;

    // Clamp the cursor position within screen bounds
    if (*cursor_x < 0) *cursor_x = 0;
    if (*cursor_x >= FRAMEBUFFER_WIDTH) *cursor_x = FRAMEBUFFER_WIDTH - 1;
    if (*cursor_y < 0) *cursor_y = 0;
    if (*cursor_y >= FRAMEBUFFER_HEIGHT) *cursor_y = FRAMEBUFFER_HEIGHT - 1;
}

void mudclient_start_application(mudclient *mud, char *title) {
#ifdef DREAMCAST
    vid_init(DM_640x480, PM_RGB565);
    pvr_init_defaults();
    pvr_set_bg_color(0.0f, 0.0f, 0.0f);
    snd_stream_init();
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
        case SDLK_TAB: 
            *code = K_TAB; 
            *char_code = '\t'; 
            break;
        case SDLK_BACKSPACE: 
            *code = K_BACKSPACE; 
            *char_code = '\b'; 
            break;
        case SDLK_LEFT: 
            *code = K_LEFT; 
            break;
        case SDLK_RIGHT: 
            *code = K_RIGHT; 
            break;
        case SDLK_UP: 
            *code = K_UP; 
            break;
        case SDLK_DOWN: 
            *code = K_DOWN; 
            break;
        case SDLK_PAGEUP: 
            *code = K_PAGE_UP; 
            break;
        case SDLK_PAGEDOWN: 
            *code = K_PAGE_DOWN; 
            break;
        case SDLK_HOME: 
            *code = K_HOME; 
            break;
        case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5:
        case SDLK_F6: case SDLK_F7: case SDLK_F8: case SDLK_F9:
        case SDLK_F10: case SDLK_F11: case SDLK_F12:
            *code = keysym->sym; 
            break; // Function keys
        case SDLK_ESCAPE: 
            *code = K_ESCAPE; 
            break;
        case SDLK_RETURN: 
            *code = K_ENTER; 
            *char_code = '\r'; 
            break;
        case SDLK_SPACE: 
            *code = ' '; 
            *char_code = ' '; 
            break;
        case SDLK_COMMA: 
            *code = ','; 
            *char_code = ','; 
            break;
        case SDLK_PERIOD: 
            *code = '.'; 
            *char_code = '.'; 
            break;
        case SDLK_QUOTE: 
            *code = '\''; 
            *char_code = '\''; 
            break;
        case SDLK_BACKQUOTE: 
            *code = '`'; 
            *char_code = '`'; 
            break;
        case SDLK_CAPSLOCK: 
            *code = KMOD_CAPS; 
            break;
        case SDLK_LSHIFT: case SDLK_RSHIFT: 
            *code = KMOD_SHIFT; 
            break;

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

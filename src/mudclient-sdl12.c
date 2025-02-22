#include "mudclient.h"
#include <kos.h>
//#include <dc/pvr.h>
//#include <dc/video.h>
//#include <dc/sound/stream.h>
#include <SDL/SDL.h>
#include <ctype.h>
#include <kos/dbglog.h>

// Framebuffer dimensions
#define FRAMEBUFFER_WIDTH 320
#define FRAMEBUFFER_HEIGHT 240

// Keyboard Buffer
#define DREAMCAST_KEYBOARD_BUFFER_SIZE 64

// D-Pad movement speed (pixels per frame)
#define DPAD_SPEED 5

// We keep these static variables for tracking mouse position
static int s_cursor_x = FRAMEBUFFER_WIDTH;
static int s_cursor_y = FRAMEBUFFER_HEIGHT;

void append_to_keyboard_buffer(char c);
void process_backspace(void);

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
                printf("xrel=%d, yrel=%d\n", event.motion.xrel, event.motion.yrel); //uncomment to spam mouse coords if you're into that
/*                s_cursor_x += event.motion.xrel;
                s_cursor_y += event.motion.yrel;
                mudclient_mouse_moved(mud, s_cursor_x, s_cursor_y);*/
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

    // Final boundary check after all events TODO: See if we even need this
/*    if (s_cursor_x < 0) s_cursor_x = 0;
    if (s_cursor_x >= FRAMEBUFFER_WIDTH)  s_cursor_x = FRAMEBUFFER_WIDTH - 1;
    if (s_cursor_y < 0) s_cursor_y = 0;
    if (s_cursor_y >= FRAMEBUFFER_HEIGHT) s_cursor_y = FRAMEBUFFER_HEIGHT - 1;*/
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

void mudclient_start_application(mudclient *mud, char *title) {
    dbglog(DBG_INFO, "Mudclient initialized for Dreamcast\n");
    int init = SDL_INIT_VIDEO | SDL_INIT_TIMER;
    if (SDL_Init(init) < 0) {
        mud_error("SDL_Init(): %s\n", SDL_GetError());
        exit(1);
    }

    SDL_EnableUNICODE(1);
    SDL_WM_SetCaption(title, NULL);

#ifdef RENDER_SW
    mud->screen = SDL_SetVideoMode(mud->game_width, mud->game_height, 16,
                                   SDL_SWSURFACE | SDL_FULLSCREEN);
#elif defined(RENDER_GL)
    mud->screen = SDL_SetVideoMode(mud->game_width, mud->game_height, 16,
                                   SDL_OPENGL | SDL_FULLSCREEN);
//#endif
#endif
}

#ifdef SDL12
void get_sdl_keycodes(SDL_keysym *keysym, char *char_code, int *code) {
    *code = -1;
    *char_code = -1;

    // Check for printable characters directly
    if (keysym->unicode > 0 && keysym->unicode < 128 && isprint((unsigned char)keysym->unicode)) {
        *code = keysym->unicode;
        *char_code = keysym->unicode;
        return;
    }

    // Handle specific keys
    switch (keysym->sym) {
        case SDLK_TAB: 
            *code = K_TAB; 
            *char_code = '\t'; 
            break;
        case SDLK_BACKSPACE: 
            *code = K_BACKSPACE; 
            *char_code = '\b'; 
            break;
        case SDLK_RETURN: 
            *code = K_ENTER; 
            *char_code = '\r'; 
            break;
        case SDLK_ESCAPE: 
            *code = K_ESCAPE; 
            break;

        // Arrow keys
        case SDLK_LEFT: case SDLK_RIGHT: case SDLK_UP: case SDLK_DOWN:
            *code = keysym->sym;
            break;

        // Function keys
        case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5:
        case SDLK_F6: case SDLK_F7: case SDLK_F8: case SDLK_F9:
        case SDLK_F10: case SDLK_F11: case SDLK_F12:
            *code = keysym->sym;
            break;

        // Alphabet (upper-case and lower-case)
        case SDLK_a: case SDLK_b: case SDLK_c: case SDLK_d: case SDLK_e:
        case SDLK_f: case SDLK_g: case SDLK_h: case SDLK_i: case SDLK_j:
        case SDLK_k: case SDLK_l: case SDLK_m: case SDLK_n: case SDLK_o:
        case SDLK_p: case SDLK_q: case SDLK_r: case SDLK_s: case SDLK_t:
        case SDLK_u: case SDLK_v: case SDLK_w: case SDLK_x: case SDLK_y:
        case SDLK_z:
            *char_code = (keysym->mod & KMOD_SHIFT) ? toupper(keysym->sym) : tolower(keysym->sym);
            *code = keysym->sym;
            break;

        // Numeric and shifted symbols
        case SDLK_1:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '!' : '1';
            *code = keysym->sym;
            break;
        case SDLK_2:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '@' : '2';
            *code = keysym->sym;
            break;
        case SDLK_3:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '#' : '3';
            *code = keysym->sym;
            break;
        case SDLK_4:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '$' : '4';
            *code = keysym->sym;
            break;
        case SDLK_5:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '%' : '5';
            *code = keysym->sym;
            break;
        case SDLK_6:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '^' : '6';
            *code = keysym->sym;
            break;
        case SDLK_7:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '&' : '7';
            *code = keysym->sym;
            break;
        case SDLK_8:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '*' : '8';
            *code = keysym->sym;
            break;
        case SDLK_9:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '(' : '9';
            *code = keysym->sym;
            break;
        case SDLK_0:
            *char_code = (keysym->mod & KMOD_SHIFT) ? ')' : '0';
            *code = keysym->sym;
            break;

        // Punctuation and symbols
        case SDLK_MINUS:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '_' : '-';
            *code = keysym->sym;
            break;
        case SDLK_EQUALS:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '+' : '=';
            *code = keysym->sym;
            break;
        case SDLK_BACKQUOTE:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '~' : '`';
            *code = keysym->sym;
            break;
        case SDLK_LEFTBRACKET:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '{' : '[';
            *code = keysym->sym;
            break;
        case SDLK_RIGHTBRACKET:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '}' : ']';
            *code = keysym->sym;
            break;
        case SDLK_BACKSLASH:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '|' : '\\';
            *code = keysym->sym;
            break;
        case SDLK_SEMICOLON:
            *char_code = (keysym->mod & KMOD_SHIFT) ? ':' : ';';
            *code = keysym->sym;
            break;
        case SDLK_QUOTE:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '"' : '\'';
            *code = keysym->sym;
            break;
        case SDLK_COMMA:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '<' : ',';
            *code = keysym->sym;
            break;
        case SDLK_PERIOD:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '>' : '.';
            *code = keysym->sym;
            break;
        case SDLK_SLASH:
            *char_code = (keysym->mod & KMOD_SHIFT) ? '?' : '/';
            *code = keysym->sym;
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

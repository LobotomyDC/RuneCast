#include "mudclient.h"
#include <SDL/SDL.h>
#include <ctype.h>
#include <kos/dbglog.h>

// Keyboard Buffer
#define DREAMCAST_KEYBOARD_BUFFER_SIZE 64

// D-Pad movement speed
#define DPAD_SPEED 5

void append_to_keyboard_buffer(char c);
void process_backspace(void);

void mudclient_poll_events(mudclient *mud) {
    SDL_Event event;
    mud->mouse_scroll_delta = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {

            case SDL_QUIT:
                exit(0);
                break;

            case SDL_KEYDOWN: {
                char char_code = -1;
                int  code      = -1;
                get_sdl_keycodes(&event.key.keysym, &char_code, &code);
                if (code != -1) {
//                    dbglog(DBG_INFO, "Key pressed: %d (%c)\n", code, char_code);
                    if (char_code != -1 && isprint(char_code)) {
                        append_to_keyboard_buffer(char_code);
                    } else if (code == K_BACKSPACE) {
                        process_backspace();
                    }
                    mudclient_key_pressed(mud, code, char_code);
                }
            } break;

            case SDL_KEYUP: {
                char char_code = -1;
                int  code      = -1;
                get_sdl_keycodes(&event.key.keysym, &char_code, &code);
                if (code != -1) {
//                    dbglog(DBG_INFO, "Key released: %d\n", code);
                    mudclient_key_released(mud, code);
                }
            } break;

            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button) {
                  case 0:  // wheel up
                    mud->mouse_scroll_delta += +1;
                    break;

                  case 4:  // wheel down
                    mud->mouse_scroll_delta += -1;
                    break;

                  default: // mouse button click
                    mudclient_mouse_pressed(
                        mud,
                        event.button.x,
                        event.button.y,
                        event.button.button
                    );
                    break;
                }
                 break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button >= 1 && event.button.button <= 3) {
                    mudclient_mouse_released(
                        mud,
                        event.button.x,
                        event.button.y,
                        event.button.button
                    );
                }
                break;

/* FIXME            case SDL_JOYAXISMOTION:
                if (event.jaxis.axis == 0) {
                    total_dx += (int)((event.jaxis.value / 32767.0f) * DPAD_SPEED);
                } else if (event.jaxis.axis == 1) {
                    total_dy += (int)((event.jaxis.value / 32767.0f) * DPAD_SPEED);
                }
                break;*/

            case SDL_JOYBUTTONDOWN:
                mudclient_mouse_pressed(
                    mud,
                    mud->mouse_x,
                    mud->mouse_y,
                    event.jbutton.button == 0 ? 1 : 3
                );
                break;

            case SDL_JOYBUTTONUP:
                mudclient_mouse_released(
                    mud,
                    mud->mouse_x,
                    mud->mouse_y,
                    event.jbutton.button == 0 ? 1 : 3
                );
                break;

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

void mudclient_start_application(mudclient *mud, char *title) {
    dbglog(DBG_INFO, "Mudclient initialized for Dreamcast\n");


    const int init = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER;   
    if (SDL_Init(init) < 0) {
        mud_error("SDL_Init(): %s\n", SDL_GetError());
        exit(1);
    }


    if (SDL_NumJoysticks() > 0) {
        SDL_JoystickOpen(0);
        SDL_JoystickEventState(SDL_ENABLE);      // turn on stick events
    }

//    SDL_DC_EmulateMouse(SDL_TRUE);
    SDL_ShowCursor(SDL_ENABLE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_EnableUNICODE(1);
    SDL_WM_SetCaption(title, NULL);

#ifdef RENDER_SW
    mud->screen = SDL_SetVideoMode(
        mud->game_width, mud->game_height, 16,
        SDL_SWSURFACE | SDL_FULLSCREEN
    );
#elif defined(RENDER_GL)
    mud->screen = SDL_SetVideoMode(
        mud->game_width, mud->game_height, 16,
        SDL_OPENGL | SDL_FULLSCREEN
    );
#endif
mud->mouse_x = MUD_WIDTH  / 2;
mud->mouse_y = MUD_HEIGHT / 2;
}

#ifdef SDL12
void get_sdl_keycodes(SDL_keysym *keysym, char *char_code, int *code) {
    *code = -1;
    *char_code = -1;

    if (keysym->sym == SDLK_LSHIFT || keysym->sym == SDLK_RSHIFT)
        return;

    switch (keysym->sym) {
        case SDLK_TAB:       *code=K_TAB;       *char_code='\t'; return;
        case SDLK_BACKSPACE: *code=K_BACKSPACE; *char_code='\b'; return;
        case SDLK_RETURN:    *code=K_ENTER;     *char_code='\r'; return;
        case SDLK_ESCAPE:    *code=K_ESCAPE;    /*no char*/      return;
        case SDLK_SPACE:     *code=SDLK_SPACE;  *char_code=' ';  return;

        // Arrow and Function keys
        case SDLK_LEFT: case SDLK_RIGHT: case SDLK_UP: case SDLK_DOWN:
        case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5:
        case SDLK_F6: case SDLK_F7: case SDLK_F8: case SDLK_F9:
        case SDLK_F10: case SDLK_F11: case SDLK_F12:
            *code = keysym->sym;
            return;

        default:
            break;
    }

    // Letters and digits
        if ((keysym->sym >= SDLK_a && keysym->sym <= SDLK_z) ||
            (keysym->sym >= SDLK_0 && keysym->sym <= SDLK_9)) {

        // Letters a–z
        if (keysym->sym >= SDLK_a) {
            *char_code = (keysym->mod & KMOD_SHIFT)
                ? toupper(keysym->sym)
                : tolower(keysym->sym);

        // Digits 0–9, with shifted symbols
        } else {
            static const char shifted_map[10] = {
                ')','!','@','#','$','%','^','&','*','('
            };
            int idx = keysym->sym - SDLK_0;
            *char_code = (keysym->mod & KMOD_SHIFT)
                ? shifted_map[idx]
                : (char)('0' + idx);
        }

        *code = keysym->sym;
        return;
    }

    // 3) Punctuation
    switch (keysym->sym) {
      case SDLK_COMMA:        *char_code = (keysym->mod & KMOD_SHIFT) ? '<' : ',';  *code = SDLK_COMMA;        return;
      case SDLK_PERIOD:       *char_code = (keysym->mod & KMOD_SHIFT) ? '>' : '.';  *code = SDLK_PERIOD;       return;
      case SDLK_SLASH:        *char_code = (keysym->mod & KMOD_SHIFT) ? '?' : '/';  *code = SDLK_SLASH;        return;
      case SDLK_SEMICOLON:    *char_code = (keysym->mod & KMOD_SHIFT) ? ':' : ';';  *code = SDLK_SEMICOLON;    return;
      case SDLK_QUOTE:        *char_code = (keysym->mod & KMOD_SHIFT) ? '"' : '\''; *code = SDLK_QUOTE;        return;
      case SDLK_LEFTBRACKET:  *char_code = (keysym->mod & KMOD_SHIFT) ? '{' : '[';  *code = SDLK_LEFTBRACKET;  return;
      case SDLK_RIGHTBRACKET: *char_code = (keysym->mod & KMOD_SHIFT) ? '}' : ']';  *code = SDLK_RIGHTBRACKET; return;
      case SDLK_BACKSLASH:    *char_code = (keysym->mod & KMOD_SHIFT) ? '|' : '\\'; *code = SDLK_BACKSLASH;    return;
      case SDLK_MINUS:        *char_code = (keysym->mod & KMOD_SHIFT) ? '_' : '-';  *code = SDLK_MINUS;        return;
      case SDLK_EQUALS:       *char_code = (keysym->mod & KMOD_SHIFT) ? '+' : '=';  *code = SDLK_EQUALS;       return;
      case SDLK_BACKQUOTE:    *char_code = (keysym->mod & KMOD_SHIFT) ? '~' : '`';  *code = SDLK_BACKQUOTE;    return;
      default:
          break;
    }

    // 4) Numpad 0–9
    if (keysym->sym >= SDLK_KP0 && keysym->sym <= SDLK_KP9) {
        int idx = keysym->sym - SDLK_KP0;
        *char_code = (char)('0' + idx);
        *code      = keysym->sym;
        return;
    }

    // 5) Numpad operators
    switch (keysym->sym) {
      case SDLK_KP_DIVIDE:   *char_code = '/';  *code = SDLK_KP_DIVIDE;   return;
      case SDLK_KP_MULTIPLY: *char_code = '*';  *code = SDLK_KP_MULTIPLY; return;
      case SDLK_KP_MINUS:    *char_code = '-';  *code = SDLK_KP_MINUS;    return;
      case SDLK_KP_PLUS:     *char_code = '+';  *code = SDLK_KP_PLUS;     return;
      case SDLK_KP_PERIOD:   *char_code = '.';  *code = SDLK_KP_PERIOD;   return;
      default:
          break;
    }

    // 6) Finally, use SDL unicode for anything left (if your build supports it)
    if (keysym->unicode > 0 && isprint((unsigned char)keysym->unicode)) {
        *char_code = (char)keysym->unicode;
        *code      = keysym->sym;
        return;
    }
}
#endif

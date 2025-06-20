#include "mudclient.h"
#include <SDL/SDL.h>
#include <ctype.h>
#include <kos/dbglog.h>
#include <SDL/SDL_dreamcast.h>

static SDL_Joystick *g_joy;

#define JOYSTICK_MOUSE_SPEED   2.5f
#define DPAD_MOUSE_SPEED 8
#define JOYSTICK_DEADZONE      100

/*static int mousemotion_count = 0;
static uint32_t mousemotion_last_print = 0;*/

void draw_cursor(SDL_Surface *screen, int x, int y) { //making a new cursor because we're trying to use a hwsurface and SDL's cursor is sw-only
    SDL_Rect rect = { x, y, 8, 8 };
    Uint32 white = SDL_MapRGB(screen->format, 255, 255, 255);
    SDL_FillRect(screen, &rect, white);
}

void mudclient_poll_events(mudclient *mud) {
    SDL_Event event;
    mud->mouse_scroll_delta = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {

            case SDL_QUIT:
                exit(0);
                break;

            case SDL_KEYDOWN: {
                SDLKey sym = event.key.keysym.sym;
                char cc = -1;
                int code = -1;
                get_sdl_keycodes(&event.key.keysym, &cc, &code);
                //printf("SDL sym=%d  →  mapped code=%d\n", (int)sym, code);
                if (code != -1) {
                    mudclient_key_pressed(mud, code, cc);
                }
            } break;

            case SDL_KEYUP: {
                char char_code = -1;
                int  code      = -1;
                get_sdl_keycodes(&event.key.keysym, &char_code, &code);
                if (code != -1) {
                    mudclient_key_released(mud, code);
                }
            } break;

            case SDL_MOUSEMOTION: {
/*                mousemotion_count++;
                uint32_t now = SDL_GetTicks();
                if (now - mousemotion_last_print > 1000) {
                    printf("SDL_MOUSEMOTION events: %d per second\n", mousemotion_count);
                    mousemotion_count = 0;
                    mousemotion_last_print = now;
                }

                printf("SDL_MOUSEMOTION: x=%d y=%d xrel=%d yrel=%d\n",
                    event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);*/

                mud->mouse_x = event.motion.x;
                mud->mouse_y = event.motion.y;
                // Clamp
/*                if (mud->mouse_x < 0) mud->mouse_x = 0;
                if (mud->mouse_x >= MUD_WIDTH) mud->mouse_x = MUD_WIDTH - 1;
                if (mud->mouse_y < 0) mud->mouse_y = 0;
                if (mud->mouse_y >= MUD_HEIGHT) mud->mouse_y = MUD_HEIGHT - 1;
                mudclient_mouse_moved(mud, mud->mouse_x, mud->mouse_y);*/
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                printf("MOUSEBUTTONDOWN: button=%d x=%d y=%d\n", event.button.button, event.button.x, event.button.y);
                if (event.button.button <= 2) { // 0=left, 1=right, 2=middle
                    mudclient_mouse_pressed(
                        mud,
                        event.button.x,
                        event.button.y,
                        event.button.button
                    );
                }
                switch (event.button.button) {
                  case 4:  // wheel up
                    mud->mouse_scroll_delta += +1;
                    break;

                  case 5:  // wheel down
                    mud->mouse_scroll_delta += -1;
                    break;
                }
                 break;

            case SDL_MOUSEBUTTONUP:
                    printf("MOUSEBUTTONUP: button=%d x=%d y=%d\n", event.button.button, event.button.x, event.button.y);
                if (event.button.button <= 2) { // 0=left, 1=right, 2=middle
                    mudclient_mouse_released(
                        mud,
                        event.button.x,
                        event.button.y,
                        event.button.button
                    );
                }
                break;

            //D-Pad
            case SDL_JOYHATMOTION:
/*                if (event.jhat.hat == 0) { //D-PAD buttons are Mouse Cartesian Movement
                    int moved = 0;
                    if (event.jhat.value & SDL_HAT_LEFT) {
                        mud->mouse_x -= DPAD_MOUSE_SPEED;
                        moved = 1;
                    }
                    if (event.jhat.value & SDL_HAT_RIGHT) {
                        mud->mouse_x += DPAD_MOUSE_SPEED;
                        moved = 1;
                    }
                    if (event.jhat.value & SDL_HAT_UP) {
                        mud->mouse_y -= DPAD_MOUSE_SPEED;
                        moved = 1;
                    }
                    if (event.jhat.value & SDL_HAT_DOWN) {
                        mud->mouse_y += DPAD_MOUSE_SPEED;
                        moved = 1;
                    }

                    if (moved) {
                        if (mud->mouse_x < 0) mud->mouse_x = 0;
                        if (mud->mouse_x >= MUD_WIDTH) mud->mouse_x = MUD_WIDTH - 1;
                        if (mud->mouse_y < 0) mud->mouse_y = 0;
                        if (mud->mouse_y >= MUD_HEIGHT) mud->mouse_y = MUD_HEIGHT - 1;
                        mudclient_mouse_moved(mud, mud->mouse_x, mud->mouse_y);
                        SDL_WarpMouse(mud->mouse_x, mud->mouse_y);
                    }
                }
                break;*/

                if (event.jhat.hat == 0) { //D-PAD buttons are Arrow Keys
                    // left/right on d‐pad → rotate
                    if (event.jhat.value & SDL_HAT_LEFT)  mudclient_key_pressed(mud, K_LEFT,  0);
                    else                                   mudclient_key_released(mud, K_LEFT);
                    if (event.jhat.value & SDL_HAT_RIGHT) mudclient_key_pressed(mud, K_RIGHT, 0);
                    else                                   mudclient_key_released(mud, K_RIGHT);
                    // up/down on d‐pad → zoom
                    if (event.jhat.value & SDL_HAT_UP)    mud->mouse_scroll_delta = +1;
                    else if (event.jhat.value & SDL_HAT_DOWN) mud->mouse_scroll_delta = -1;
                }
                break;

              //Let's get some face buttons in there, too:
            case SDL_JOYBUTTONDOWN:
              switch (event.jbutton.button) {
                case 2:  // “A” → left click
                  mudclient_mouse_pressed(mud, mud->mouse_x, mud->mouse_y, 0);
                  break;
                case 1:  // “B” → right click
                  mudclient_mouse_pressed(mud, mud->mouse_x, mud->mouse_y, 1);
                  break;
                case 6:  // “X” → middle click
                case 5:  // “Y” → middle click
                  mudclient_mouse_pressed(mud, mud->mouse_x, mud->mouse_y, 2);
                  break;
              }
              break;

            case SDL_JOYBUTTONUP:
              switch (event.jbutton.button) {
                case 2:
                  mudclient_mouse_released(mud, mud->mouse_x, mud->mouse_y, 0);
                  break;
                case 1:
                  mudclient_mouse_released(mud, mud->mouse_x, mud->mouse_y, 1);
                  break;
                case 6: case 5:
                  mudclient_mouse_released(mud, mud->mouse_x, mud->mouse_y, 2);
                  break;
              }
          }
      }
    if (g_joy) {
        SDL_JoystickUpdate();

        // left stick controls mouse cursor
        Sint16 lx = SDL_JoystickGetAxis(g_joy, 0);
        Sint16 ly = SDL_JoystickGetAxis(g_joy, 1);
        if (abs(lx) > JOYSTICK_DEADZONE || abs(ly) > JOYSTICK_DEADZONE) {
            float fx = lx / 32767.0f;
            float fy = ly / 32767.0f;
            mud->mouse_x += (int)(fx * JOYSTICK_MOUSE_SPEED);
            mud->mouse_y -= (int)(fy * JOYSTICK_MOUSE_SPEED);


            // clamp
            if (mud->mouse_x < 0) mud->mouse_x = 0;
            if (mud->mouse_x >= MUD_WIDTH)  mud->mouse_x = MUD_WIDTH - 1;
            if (mud->mouse_y < 0) mud->mouse_y = 0;
            if (mud->mouse_y >= MUD_HEIGHT) mud->mouse_y = MUD_HEIGHT - 1;

            mudclient_mouse_moved(mud, mud->mouse_x, mud->mouse_y);
        }

        // vertical right stick: axis 2 & 3 → camera/mouse-wheel emulation
        Sint16 rx = SDL_JoystickGetAxis(g_joy, 2);
        Sint16 ry = SDL_JoystickGetAxis(g_joy, 3);

        // horizontal right stick → map to “left arrow” / “right arrow” presses
        if      (rx < -JOYSTICK_DEADZONE) mudclient_key_pressed(mud, K_LEFT,  0);
        else if (rx >  JOYSTICK_DEADZONE) mudclient_key_pressed(mud, K_RIGHT, 0);
        else {
            mudclient_key_released(mud, K_LEFT);
            mudclient_key_released(mud, K_RIGHT);
        }

        // vertical right stick → mouse wheel
        if      (ry < -JOYSTICK_DEADZONE) mud->mouse_scroll_delta = +1;
        else if (ry >  JOYSTICK_DEADZONE) mud->mouse_scroll_delta = -1;
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
        g_joy = SDL_JoystickOpen(0);
        SDL_JoystickEventState(SDL_ENABLE);
    } else {
        g_joy = NULL;
    }
   
//    SDL_DC_EmulateMouse(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(200, 30); //TODO: Play with this
    SDL_WM_SetCaption(title, NULL);

#ifdef RENDER_SW
    mud->screen = SDL_SetVideoMode(mud->game_width, mud->game_height, 16, SDL_HWSURFACE);//SDL_VIDEO_OPENGL
#elif defined(RENDER_GL)
    mud->screen = SDL_SetVideoMode(
        mud->game_width, mud->game_height, 16,
        SDL_OPENGL | SDL_FULLSCREEN
    );
#endif
mud->mouse_x = MUD_WIDTH  / 1;
mud->mouse_y = MUD_HEIGHT / 1;
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
        case SDLK_LEFT:      *code = K_LEFT;    *char_code= -1;  return;
        case SDLK_RIGHT:     *code = K_RIGHT;   *char_code= -1;  return;
        default: break;
    }

    switch (keysym->sym) {
        case SDLK_UP:    *code = K_UP;    return;
        case SDLK_DOWN:  *code = K_DOWN;  return;

        case SDLK_F1:  case SDLK_F2:  case SDLK_F3:  case SDLK_F4:
        case SDLK_F5:  case SDLK_F6:  case SDLK_F7:  case SDLK_F8:
        case SDLK_F9:  case SDLK_F10: case SDLK_F11: case SDLK_F12:
          *code = keysym->sym;
          return;
    }

    if ((keysym->sym >= SDLK_a && keysym->sym <= SDLK_z) ||
        (keysym->sym >= SDLK_0 && keysym->sym <= SDLK_9)) {

        if (keysym->sym >= SDLK_a) {
            *char_code = (keysym->mod & KMOD_SHIFT)
                ? toupper(keysym->sym)
                : tolower(keysym->sym);
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

    //Punctuation keys. TODO: Figure out why BACKQUOTE/TILDE doesn't work. It looks like everything is set up correctly?
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
      case SDLK_PLUS:         *char_code = (keysym->mod & KMOD_SHIFT) ? '+' : '=';  *code = SDLK_PLUS;         return;
      case SDLK_BACKQUOTE:    *char_code = (keysym->mod & KMOD_SHIFT) ? '~' : '`';  *code = SDLK_BACKQUOTE;    return;
      default:
          break;
    }

    //Numpad keys
    if (keysym->sym >= SDLK_KP0 && keysym->sym <= SDLK_KP9) {
        int idx = keysym->sym - SDLK_KP0;
        *char_code = (char)('0' + idx);
        *code      = keysym->sym;
        return;
    }

    switch (keysym->sym) {
      case SDLK_KP_DIVIDE:   *char_code = '/';  *code = SDLK_KP_DIVIDE;   return;
      case SDLK_KP_MULTIPLY: *char_code = '*';  *code = SDLK_KP_MULTIPLY; return;
      case SDLK_KP_MINUS:    *char_code = '-';  *code = SDLK_KP_MINUS;    return;
      case SDLK_KP_PLUS:     *char_code = '+';  *code = SDLK_KP_PLUS;     return;
      case SDLK_KP_PERIOD:   *char_code = '.';  *code = SDLK_KP_PERIOD;   return;
      default:
          break;
    }
    
    if (keysym->unicode > 0 && isprint((unsigned char)keysym->unicode)) {
        *char_code = (char)keysym->unicode;
        *code      = keysym->sym;
        return;
    }
}
#endif

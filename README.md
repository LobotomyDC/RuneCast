# RuneCast -- An RSC-C Port to the SEGA Dreamcast

portable, enhanced runescape classic client ported to C99. supports 204 and 177
protocols. original mudclient204 java source refactored by v0rtex/xetr0v and
Isaac Eggsampler. compatible with [rscsundae](https://git.sr.ht/~stormy/rscsundae),
[openrsc](https://rsc.vet/) and 2003scape.

Prominent new features include:
* native port for Sega Dreamcast!
* Keyboard and Mouse Support!
* It doesn't run on stock hardware (yet)!

### Controls (Dreamcast):

* D-Pad/R Analog (if available) to control camera
* L analog for mouse input
* hold L whilst tapping touch screen for right click
* Start -
* select to toggle top screen's power
* A/B - mouse button 1 and 2
* X/Y - F1 and F2 for now

### Build Instructions:

Requires KallistiOS set up to build

1. Run KallistiOS environment script 
```source /opt/toolchains/dc/kos/environ.sh```

2.
```make -f Makefile.dreamcast```

3.
To clean your build environment:
```make -f Makefile.dreamcast clean```

## Build Instructions (linux)

install [libsdl2-dev](https://packages.debian.org/sid/libsdl2-dev).
if compiling with opengl support, also install
[libsdl2-image-dev](https://packages.debian.org/sid/libsdl2-image-dev),
[libglew-dev](https://packages.debian.org/sid/libglew-dev),
[libgl-dev](https://packages.debian.org/sid/libgl-dev),
and [pkgconf](https://packages.debian.org/sid/pkgconf).

    $ make
    $ ./mudclient

distribute with `./cache` directory.

### run requirements

-A Sega Dreamcast

-A computer with the KallistiOS build environment

## Credits
* @Bruceleeto for the initial porting attempt, helping me get started with a makefile, as well as a few pointers I probably should have figured out by reading the documentation
* The Simulant.dev and OpenRSC Discord Servers

## options

```
; IPv4 address with revision 177 compatible protocol support
server = game.openrsc.com
port = 43596
; Disable registration and load sounds, P2P landscape and items (requires
; restart)
members = 1
; If enabled, override members option and always show New User button
registration = 1
; Used together to encrypt passwords, Must be represented as hexadecimal string
; 0-padded to a multiple of eight characters
rsa_exponent = 00010001
rsa_modulus = 87cef754966ecb19806238d9fecf0f421e816976f74f365c86a584e51049794d41fefbdc5fed3a3ed3b7495ba24262bb7d1dd5d2ff9e306b5bbf5522a2e85b25
; Log out when mouse is idle
idle_logout = 0
; Remember username on login screen
remember_username = 0
; Remember password on login screen (not secure)
remember_password = 0

username =
password =

; System command to use to open the web browser (only on desktop)
browser_command = xdg-open "%s"

; Diversify NPCs sent by server (custom)
diversify_npcs = 0

; Rename Herblaw items for ease of identification (custom)
rename_herblaw_items = 1

; Scroll panel lists, chatbox, and camera (if zoom enabled) with wheel
mouse_wheel = 1
; Hold down middle click and move mouse to rotate camera (manual mode)
middle_click_camera = 25
; Use arrow, page, home keys and mouse wheel (if enabled) to zoom
zoom_camera = 1
; Respond to the last private message with tab key
tab_respond = 1
; Use number keys to select options
option_numbers = 1
; Adds a menu with different directions to face to the minimap compass
compass_menu = 1
; Adds right click menus to trades and duels
transaction_menus = 1
; Allow inputting item amounts
offer_x = 1
; Add another button to perform the last offer x amount
last_offer_x = 1
; Add RuneScape Wiki lookup button instead of report abuse
wiki_lookup = 1
; Combat style menu is usable outside of combat
combat_style_always = 0
; Hold to buy/sell items from shops in bulk (matches trade screen)
hold_to_buy = 1
; Drag vertically to zoom camera
touch_vertical_drag = 33
; Drag horizontally to pan camera
touch_pinch = 50
; Milliseconds until right click
touch_menu_delay = 350

; Low memory mode
lowmem = 0
; F1 mode - only render every second scanline
interlace = 0
; Underground lighting flicker
flicker = 1
; Fog of War (FoW)
fog_of_war = 1
; Target framerate of @ran@ text effect
ran_target_fps = 10
; Display the FPS at the bottom right of the screen
display_fps = 0
; Double the UI size but keep the scene size if window is over double original
; size (GL only)
ui_scale = 1
; Enable multi-sampling
anti_alias = 1
; Change the field of view. scales with height by default and in
; software. About 36 degrees on the original height of 346 (GL only)
field_of_view = 360
; Show roofs unless inside buildings
show_roofs = 1
; Format large numbers with commas
number_commas = 1
; Show the remaining experience until next level in skills tab
remaining_experience = 1
; Show your total experience in the skills tab
total_experience = 1
; Show experience drops
experience_drops = 0
; Show a count of inventory items on the UI
inventory_count = 0
; Condenses item amounts with K and M and add their amounts to examine
condense_item_amounts = 1
; Also draw which item a certificate is associated with
certificate_items = 1
; Display the warning dialog near the wilderness border
wilderness_warning = 1
; Display hits and prayer bars
status_bars = 0
; Use ground item models instead of billboarded sprites
ground_item_models = 1
; Show text for valuable ground items to make them stand out
ground_item_text = 1
; Always animate objects like fires
distant_animation = 1
; Load less compressed (2001 era) sprites
tga_sprites = 0
; Show hover tooltip menu
show_hover_tooltip = 0
; Move the keyboard button to the right
touch_keyboard_right = 0

; Add filtering to the bank
bank_search = 1
; Adds capacity to the bank
bank_capacity = 1
; Adds total high alchemy value to the bank
bank_value = 1
; Expand bank item grid with client height
bank_expand = 1
; Use a scrollbar instead of bank pages
bank_scroll = 1
; Adds right click menus to bank items
bank_menus = 1
; Shows the inventory along with the bank interface, given enough width
bank_inventory = 1
; Maintain the selected bank slot when items change position
bank_maintain_slot = 1
```

## libraries used

* [glew](http://glew.sourceforge.net/) for runtime opengl extension loading
* [ini](https://github.com/rxi/ini) for parsing *options.ini*
* [isaac](https://burtleburtle.net/bob/rand/isaacafa.html) for authentic packet
decoding
* [libsdl2](https://www.libsdl.org/index.php) for input/output on desktop
* [micro-bunzip](https://landley.net/code/) for decompressing cache archives
* [tiny-bignum-c](https://github.com/kokke/tiny-bignum-c) for RSA encryption on
login/registration

## license
Copyright 2024  2003Scape Team

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along
with this program. If not, see http://www.gnu.org/licenses/.

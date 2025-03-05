# RuneCast -- An RSC-C Port to the SEGA Dreamcast, SGI IRIX Workstations, and other Legacy Hardware!

portable, enhanced runescape classic client ported to C99. supports 204 and 177
protocols. original mudclient204 java source refactored by v0rtex/xetr0v and
Isaac Eggsampler. compatible with [rscsundae](https://git.sr.ht/~stormy/rscsundae),
[openrsc](https://rsc.vet/) and 2003scape.

### Prominent new features include:
* Native port for Sega Dreamcast!
* Modular build options! (WIP)
* Keyboard and Mouse Support!
* It doesn't run on stock hardware (yet)!
* [Discord Server](https://discord.gg/S82mFwhhEy)

### Run Requirements

* A Sega Dreamcast, or Dreamcast Emulator, such as Flycast, Devcast, or Deecy

* [A Computer with the KallistiOS build environment installed](https://dreamcast.wiki/Getting_Started_with_Dreamcast_development)

### Controls (Dreamcast):

* Maple Keyboard and Mouse

* OR (soon)

* Sega Dreamcast Controller:
* D-Pad/R Analog (if available) to control camera
* L analog for mouse input
* Start - ??? (Probably on-screen keyboard)
* A/B - mouse button 1 and 2
* X/Y - F1 and F2 for now. Will come up with better controls as I go, and try to support Dual Analog and Extended Controller Buttons.

## Build Instructions (Dreamcast):

[Requires KallistiOS set up to build](https://dreamcast.wiki/Getting_Started_with_Dreamcast_development). When that is installed, cd to the RuneCast folder in your Terminal and type the following:
 
* 1.```source /opt/toolchains/dc/kos/environ.sh``` To build with the Dreamcast Toolchain
* 2.```make -f Makefile.dreamcast [SDL-VERSION-OF-CHOICE] clean``` Clean your build environment

* This part of the process has you choose between building with SDL1.2 (Built into KallistiOS), or SDL2. Instructions on building SDL2 are below these instructions

* 3a.```make -f Makefile.dreamcast SDLKOS=1``` if building with SDL1.2 (Recommended for now)
* 3b (Optional).```make -f Makefile.dreamcast SDL2=1``` if building with SDL2 (Unfinished)

* If you want to build with GLdc (Currently unimplemented, don't bother trying just yet):
* 3c (Optional). ```make -f Makefile.dreamcast [SDL-VERSION-OF-CHOICE] GLDC=1```

* Also, If you have mkdcdisc installed, symlinked to /usr/local/bin, and want to build the game and make a CDI right away, with a single command, you can do this...
* 3d.```make -f Makefile.dreamcast [SDL-VERSION-OF-CHOICE] CDI=1```
* ...else, if you built without CDI=1, and only have an .elf...
* 4.```mkdcdisc -e mudclient.elf -d './cache' -o RuneScape.cdi -N``` 
* ...you can build the CDI yourself with that command.
* 5(optional).```flycast RuneScape.cdi``` 
* For instant testing, if you have flycast set up to run as a command by symlinking to /usr/local/bin. Other emulators may do this, I'm not sure.

## (Optional) Build Instructions (Dreamcast-SDL2):

* This is for the DC SDL2 library itself. Refer to the instructions above to actually build the game with SDL2. As SDL1.2 comes with KallistiOS as a KOS-Port, there is no need to do this for that version. As SDL2 is available as a KOS-Addon external to KallistiOS. I've included it, and build instructions, in this package, [but you should check the original repo for updates.](https://github.com/GPF/SDL2) Dreamcast SDL2 is under continuous development, and implementation is currently experimental in this project. I do not update the SDL2 that I've included in this project, and implore you to track the progress of its development, and periodically update/rebuild it as it continues.

* 1. ```source /opt/toolchains/dc/kos/environ.sh``` To build with the Dreamcast Toolchain
* 2. ```cd [PATH-TO-RUNECAST-DIRECTORY]/SDL2-dreamcastSDL2/build-scripts``` change directory to the 'build-scripts' folder
* 3. ```./dreamcast.sh``` 'Run the Dreamcast SDL2 build script.' This will automatically install the library to the addons folder in your DC toolchain.

### "Gee, Billy! How come your mom lets you have TWO SDLs?"
 I think it's useful to have the ability to pick and choose the libraries you want to use for the games you're building. I'm still learning how to port games, so having both options will not only allow me to benchmark performance, but also possibly help me find problems with the game itself. If there are other libraries I use that have "competition" (Like GLdc and KallistiGL), then I will also try to incorporate both of them as build options. Not many Dreamcast games have this option, and I think it's a good idea to get real-world performance to benchmark the libraries that we use, so that everything can be improved.

## Build Instructions (linux):

install [libsdl2-dev](https://packages.debian.org/sid/libsdl2-dev).
if compiling with opengl support, also install
[libsdl2-image-dev](https://packages.debian.org/sid/libsdl2-image-dev),
[libglew-dev](https://packages.debian.org/sid/libglew-dev),
[libgl-dev](https://packages.debian.org/sid/libgl-dev),
and [pkgconf](https://packages.debian.org/sid/pkgconf).

    $ make
    $ ./mudclient

distribute with `./cache` directory.


## Credits
* @Bruceleeto for the initial porting attempt, helping me get started with a makefile, as well as a few pointers I probably should have figured out by reading the documentation.
* Frogdoubler for answering my questions with an insane level of detail.
* The Simulant.dev and OpenRSC Discord Servers.
* @GPF/GPFTroy for the SDL2 Port to Dreamcast.

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
* [KallistiOS](https://github.com/KallistiOS/KallistiOS) The incredible Sega Dreamcast SDK that makes this possible
* [SDL2 for Sega Dreamcast](https://github.com/GPF/SDL2) for Dreamcast I/O
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

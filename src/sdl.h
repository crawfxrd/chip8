#ifndef CHIP8_SDL_H_
#define CHIP8_SDL_H_

#include "chip8.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

bool sdl_init(void);
void sdl_cleanup(void);
void sdl_render(Chip8 *chip);

#endif


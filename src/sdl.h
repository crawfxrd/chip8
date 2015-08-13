#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

bool SetupSDL(void);
void CleanupSDL(void);
void UpdateDisplay(uint8_t *);


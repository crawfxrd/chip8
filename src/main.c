#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "chip8.h"
#include "sdl.h"

static void SetKey(CHIP8 *, SDL_Keycode, bool);

int main(int argc, char *argv[])
{
    CHIP8 chip;
    SDL_Event event;

    if (argc != 2)
    {
        printf("Usage: chip8 <rom>\n");
        return 0;
    }

    srand(time(NULL));

    if (!SetupSDL())
    {
        CleanupSDL();
        return 1;
    }

    Reset(&chip);

    if (!LoadROM(&chip, argv[1]))
    {
        printf("Failed to laod ROM.\n");
        return 1;
    }

    while (!chip.Halt)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                chip.Halt = true;
                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    chip.Halt = true;
                }
                else
                {
                    SetKey(&chip, event.key.keysym.sym, false);
                }

                break;

            case SDL_KEYDOWN:
                SetKey(&chip, event.key.keysym.sym, true);
                break;
            }
        }

        FDX(&chip);

        if (chip.Draw)
        {
            UpdateDisplay(chip.Display);
            chip.Draw = false;
        }

        UpdateTimers(&chip);

        /* FIXME: Use proper timing. */
        SDL_Delay(4);
    }

    CleanupSDL();
    return 0;
}

static
void
SetKey(CHIP8 *chip, SDL_Keycode key, bool pressed)
{
    switch (key)
    {
    case SDLK_1: chip->Keypad[0x1] = pressed; break;
    case SDLK_2: chip->Keypad[0x2] = pressed; break;
    case SDLK_3: chip->Keypad[0x3] = pressed; break;
    case SDLK_4: chip->Keypad[0xC] = pressed; break;

    case SDLK_q: chip->Keypad[0x4] = pressed; break;
    case SDLK_w: chip->Keypad[0x5] = pressed; break;
    case SDLK_e: chip->Keypad[0x6] = pressed; break;
    case SDLK_r: chip->Keypad[0xD] = pressed; break;

    case SDLK_a: chip->Keypad[0x7] = pressed; break;
    case SDLK_s: chip->Keypad[0x8] = pressed; break;
    case SDLK_d: chip->Keypad[0x9] = pressed; break;
    case SDLK_f: chip->Keypad[0xE] = pressed; break;

    case SDLK_z: chip->Keypad[0xA] = pressed; break;
    case SDLK_x: chip->Keypad[0x0] = pressed; break;
    case SDLK_c: chip->Keypad[0xB] = pressed; break;
    case SDLK_v: chip->Keypad[0xF] = pressed; break;
    }
}


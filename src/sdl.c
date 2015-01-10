#include "sdl.h"
#include <stdio.h>

static bool create_window(void);
static bool create_renderer(void);
static void init_display(void);

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 320;

static SDL_Window *gWindow = NULL;
static SDL_Renderer *gRenderer = NULL;
static SDL_Rect gDisplay[64 * 32];

bool sdl_init(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    if (!create_window())
        return false;

    if (!create_renderer())
        return false;

    SDL_RenderSetLogicalSize(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

    init_display();

    return true;
}

static bool create_window(void)
{
    gWindow = SDL_CreateWindow(
            "CHIP-8",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (gWindow == NULL)
    {
        printf("Failed to create window: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

static bool create_renderer(void)
{
    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);

    if (gRenderer == NULL)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

static void init_display(void)
{
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            gDisplay[(y * 64) + x].x = (x * 10);
            gDisplay[(y * 64) + x].y = (y * 10);
            gDisplay[(y * 64) + x].w = 10;
            gDisplay[(y * 64) + x].h = 10;
        }
    }

}

void sdl_cleanup()
{
    if (gRenderer != NULL)
    {
        SDL_DestroyRenderer(gRenderer);
        gRenderer = NULL;
    }

    if (gWindow != NULL)
    {
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
    }

    SDL_Quit();
}

void sdl_render(Chip8 *chip)
{
    SDL_RenderClear(gRenderer);
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);

    //SDL_RenderFillRects(gRenderer, gDisplay, (64 * 32));
    for (int i = 0; i < (64 * 32); i++)
    {
        if (chip->graphics[i] == 1)
        {
            SDL_RenderFillRect(gRenderer, &gDisplay[i]);
        }
    }

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderPresent(gRenderer);
}


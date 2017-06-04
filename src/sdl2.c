#include "sdl2.h"

#include <stdio.h>

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 320;

static SDL_Window *gWindow = NULL;
static SDL_Renderer *gRenderer = NULL;
static SDL_Texture *gTexture = NULL;

static
bool
CreateWindow(void)
{
    gWindow = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (gWindow == NULL)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

static
bool
CreateRenderer(void)
{
    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);

    if (gRenderer == NULL)
    {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

static
bool
CreateTexture(void)
{
    gTexture = SDL_CreateTexture(
        gRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32);

    if (gTexture == NULL)
    {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool
SetupSDL(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    if (!CreateWindow())
        return false;

    if (!CreateRenderer())
        return false;

    if (!CreateTexture())
        return false;

    SDL_RenderSetLogicalSize(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    return true;
}

void
CleanupSDL(void)
{
    if (gTexture != NULL)
    {
        SDL_DestroyTexture(gTexture);
        gTexture = NULL;
    }

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

void
UpdateDisplay(uint8_t *display)
{
    void *pixels;
    int pitch;

    SDL_LockTexture(gTexture, NULL, &pixels, &pitch);

    for (int i = 0; i < (64 * 32); i++)
    {
        *(((int *)pixels) + i) = display[i] ? -1: 0;
    }

    SDL_UnlockTexture(gTexture);

    SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    SDL_RenderPresent(gRenderer);
}


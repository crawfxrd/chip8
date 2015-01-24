#include "graphics.h"
#include <SDL2/SDL.h>

namespace
{
    const int kScreenWidth = 640;
    const int kScreenHeight = 320;
}

Graphics::Graphics()
{
    window_ = SDL_CreateWindow(
            "CHIP-8",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            kScreenWidth, kScreenHeight, 0);

    renderer_ = SDL_CreateRenderer(window_, -1, 0);

    SDL_RenderSetLogicalSize(renderer_, kScreenWidth, kScreenHeight);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
}

Graphics::~Graphics()
{
    if (renderer_ != nullptr)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void Graphics::Clear()
{
    SDL_RenderClear(renderer_);
}

void Graphics::Render()
{
    SDL_RenderPresent(renderer_);
}


void Graphics::FillRects(const uint8_t p[64 * 32])
{
    SDL_Rect rect = {0, 0, 10, 10};

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            rect.x = x * 10;
            rect.y = y * 10;

            if (p[(y * 64) + x] == 1)
                SDL_RenderFillRect(renderer_, &rect);
        }
    }

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
}


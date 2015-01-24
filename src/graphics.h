#ifndef CHIP8_GRAPHICS_H_
#define CHIP8_GRAPHICS_H_

#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;

struct Graphics
{
    Graphics();
    ~Graphics();

    void Clear();
    void Render();
    void FillRects(const uint8_t p[64 * 32]);

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
};

#endif


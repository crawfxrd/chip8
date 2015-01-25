#ifndef CHIP8_GRAPHICS_H_
#define CHIP8_GRAPHICS_H_

#include <array>
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;

struct Graphics
{
    Graphics();
    ~Graphics();

    void Clear();
    void Render();
    void FillRects(const std::array<uint8_t, (64 * 32)> (&p));

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
};

#endif


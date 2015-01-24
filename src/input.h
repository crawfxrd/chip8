#ifndef CHIP8_INPUT_H_
#define CHIP8_INPUT_H_

#include <SDL2/SDL.h>
#include <map>
#include <cstdint>

struct Input
{
    void KeyDownEvent(const SDL_Event& event);
    void KeyUpEvent(const SDL_Event& event);
    bool KeyPressed(SDL_Keycode key);
    uint8_t WaitKey();
    SDL_Keycode Translate(uint8_t key);
    uint8_t Translate(SDL_Keycode key);

private:
    std::map<SDL_Keycode, bool> keys_;

    bool IsValidKey(SDL_Keycode key);
};

#endif


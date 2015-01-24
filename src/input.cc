#include "input.h"

void Input::KeyDownEvent(const SDL_Event& event)
{
    keys_[event.key.keysym.sym] = true;
}

void Input::KeyUpEvent(const SDL_Event& event)
{
    keys_[event.key.keysym.sym] = false;
}

bool Input::KeyPressed(SDL_Keycode key)
{
    return keys_[key];
}

uint8_t Input::WaitKey()
{
    SDL_Event event;

    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                KeyDownEvent(event);
                if (IsValidKey(event.key.keysym.sym))
                    return Translate(event.key.keysym.sym);
                break;

            case SDL_KEYUP:
                KeyUpEvent(event);
                break;
            }
        }
    }
}

bool Input::IsValidKey(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_q:
    case SDLK_w:
    case SDLK_e:
    case SDLK_r:
    case SDLK_a:
    case SDLK_s:
    case SDLK_d:
    case SDLK_f:
    case SDLK_z:
    case SDLK_x:
    case SDLK_c:
    case SDLK_v:
        return true;

    default:
        return false;
    }
}

SDL_Keycode Input::Translate(uint8_t key)
{
    switch (key)
    {
    case 0x0: return SDLK_x;
    case 0x1: return SDLK_1;
    case 0x2: return SDLK_2;
    case 0x3: return SDLK_3;

    case 0x4: return SDLK_q;
    case 0x5: return SDLK_w;
    case 0x6: return SDLK_e;
    case 0x7: return SDLK_a;

    case 0x8: return SDLK_s;
    case 0x9: return SDLK_d;
    case 0xA: return SDLK_z;
    case 0xB: return SDLK_c;

    case 0xC: return SDLK_4;
    case 0xD: return SDLK_r;
    case 0xE: return SDLK_f;
    case 0xF: return SDLK_v;
    }
}

uint8_t Input::Translate(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_1: return 0x1;
    case SDLK_2: return 0x2;
    case SDLK_3: return 0x3;
    case SDLK_4: return 0xC;

    case SDLK_q: return 0x4;
    case SDLK_w: return 0x5;
    case SDLK_e: return 0x6;
    case SDLK_r: return 0xD;

    case SDLK_a: return 0x7;
    case SDLK_s: return 0x8;
    case SDLK_d: return 0x9;
    case SDLK_f: return 0xE;

    case SDLK_z: return 0xA;
    case SDLK_x: return 0x0;
    case SDLK_c: return 0xB;
    case SDLK_v: return 0xF;
    }
}


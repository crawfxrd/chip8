#ifndef CHIP8_CHIP8_H_
#define CHIP8_CHIP8_H_

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define N0(x) ((x) & 0xF)
#define N1(x) (((x) >> 4) & 0xF)
#define N2(x) (((x) >> 8) & 0xF)
#define N3(x) (((x) >> 12) & 0xF)

typedef struct tagChip8
{
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t graphics[64 * 32];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    int16_t sp;
    bool draw_flag;
    bool halt;
    bool keypad[16];
    SDL_Event event;
} Chip8;

void chip8_init(Chip8 *chip);
void chip8_dump(Chip8 *chip);
bool chip8_load(Chip8 *chip, const char *rom);
void chip8_run(Chip8 *chip);
uint8_t chip8_getkey(Chip8 *chip);

#endif


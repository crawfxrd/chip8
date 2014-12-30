#ifndef CHIP8_CHIP8_H_
#define CHIP8_CHIP8_H_

#include <stdint.h>

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
} Chip8;

void chip8_init(Chip8 *chip);
void chip8_dump(Chip8 *chip);
int chip8_load(Chip8 *chip, const char *rom);

#endif


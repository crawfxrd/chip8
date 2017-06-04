#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct tagChip8 {
    uint8_t Memory[4096];
    uint8_t Display[64 * 32];
    bool Keypad[16];

    uint8_t V[16];
    uint16_t I;
    uint8_t DelayTimer;
    uint8_t SoundTimer;

    uint16_t PC;
    int8_t SP;
    uint16_t Stack[16];

    bool Draw;
    bool Halt;
} CHIP8;

void Reset(CHIP8 *);
bool LoadROM(CHIP8 *, const char *rom);
void UpdateTimers(CHIP8 *);
void FDX(CHIP8 *);

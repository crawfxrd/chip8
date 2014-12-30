#include "chip8.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static const uint8_t chip8_fontset[80] =
{
    0x60, 0x90, 0x90, 0x90, 0x60,   // 0
    0x20, 0x60, 0x20, 0x20, 0x70,   // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
    0xF0, 0x10, 0x70, 0x10, 0xF0,   // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
    0xF0, 0x90, 0xF0, 0x10, 0x10,   // 9
    0x60, 0x90, 0xF0, 0x90, 0x90,   // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
    0x60, 0x90, 0x80, 0x90, 0x60,   // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
    0xF0, 0x80, 0xE0, 0x80, 0xF0,   // E
    0xF0, 0x80, 0xE0, 0x80, 0x80,   // F
};

void chip8_init(Chip8 *chip)
{
    chip->pc = 0x200;
    chip->I = 0;
    chip->sp = -1;
    chip->delay_timer = 0;
    chip->sound_timer = 0;

    memset(chip->graphics, 0, sizeof(chip->graphics));
    memset(chip->stack, 0, sizeof(chip->stack));
    memset(chip->V, 0, sizeof(chip->V));
    memset(chip->memory, 0, sizeof(chip->memory));

    /* Load the fontset. */
    for (int i = 0; i < 80; i++)
        chip->memory[i] = chip8_fontset[i];
}

void chip8_dump(Chip8 *chip)
{
    printf("pc = 0x%" PRIX16 "\n", chip->pc);
    printf("I = %" PRIu16 "\n", chip->I);
    printf("delay_timer = %" PRIu8 "\n", chip->delay_timer);
    printf("sound_timer = %"PRIu8 "\n", chip->sound_timer);

    printf("\n");
    printf("sp = %" PRId16 "\n", chip->sp);
    for (int i = 0; i < 16; i++)
        printf("stack[%1X] = %" PRIu16 "\n", i, chip->stack[i]);

    printf("\n");
    for (int i = 0; i < 16; i++)
        printf("V[%1X] = %" PRIu8 "\n", i, chip->V[i]);

    printf("\n");
}

int chip8_load(Chip8 *chip, const char *rom)
{
    FILE *fp;
    long size;

    fp = fopen(rom, "rb");
    if (fp == NULL)
    {
        printf("Unable to open file %s\n", rom);
        return 1;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    if (size > (0x1000 - 0x200))
    {
        puts("ROM too big");
        fclose(fp);
        return 2;
    }

    for (int i = 0; i < size; i++)
    {
        chip->memory[i + 0x200] = (uint8_t)fgetc(fp);
    }

    fclose(fp);
    return 0;
}


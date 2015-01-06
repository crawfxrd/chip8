#include "chip8.h"
#include "opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

static const uint8_t chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
    0x20, 0x60, 0x20, 0x20, 0x70,   // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,   // F
};

static void chip8_cycle(Chip8 *chip);

void chip8_init(Chip8 *chip)
{
    chip->pc = 0x200;
    chip->I = 0;
    chip->sp = -1;
    chip->delay_timer = 0;
    chip->sound_timer = 0;
    chip->draw_flag = false;
    chip->halt = false;

    memset(chip->graphics, 0, sizeof(chip->graphics));
    memset(chip->stack, 0, sizeof(chip->stack));
    memset(chip->V, 0, sizeof(chip->V));
    memset(chip->memory, 0, sizeof(chip->memory));

    /* Load the fontset. */
    for (int i = 0; i < 80; i++)
        chip->memory[i] = chip8_fontset[i];

    srand(time(NULL));
}

void chip8_dump(Chip8 *chip)
{
    printf("pc = 0x%" PRIX16 "\n", chip->pc);
    printf("I = %" PRIu16 "\n", chip->I);
    printf("delay_timer = %" PRIu8 "\n", chip->delay_timer);
    printf("sound_timer = %"PRIu8 "\n", chip->sound_timer);
    printf("draw_flag = %s\n", chip->draw_flag ? "true" : "false");

    printf("\n");
    printf("sp = %" PRId16 "\n", chip->sp);
    for (int i = 0; i < 16; i++)
        printf("stack[%1X] = %" PRIu16 "\n", i, chip->stack[i]);

    printf("\n");
    for (int i = 0; i < 16; i++)
        printf("V[%1X] = %" PRIu8 "\n", i, chip->V[i]);

    printf("\n");
}

bool chip8_load(Chip8 *chip, const char *rom)
{
    FILE *fp;
    long size;

    fp = fopen(rom, "rb");
    if (fp == NULL)
    {
        printf("Unable to open file %s\n", rom);
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    if (size > (0x1000 - 0x200))
    {
        puts("ROM too big");
        fclose(fp);
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        chip->memory[i + 0x200] = (uint8_t)fgetc(fp);
    }

    fclose(fp);
    return true;
}

void chip8_run(Chip8 *chip)
{
    while(!(chip->halt))
    {
        chip8_cycle(chip);

        if (chip->draw_flag)
        {
            printf("Redraw requested at pc=0x%03X\n", chip->pc);
            chip->draw_flag = false;
        }

        /* Update timers. */
        if (chip->delay_timer > 0)
            (chip->delay_timer)--;

        if (chip->sound_timer > 0)
            (chip->sound_timer)--;
    }
}

static void chip8_cycle(Chip8 *chip)
{
    uint16_t opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];

    switch (N3(opcode))
    {
    case 0x0:
        switch (opcode & 0xFFF)
        {
        case 0x0E0:
            CLS(chip);
            break;

        case 0x0EE:
            RET(chip);
            break;

        default:
            SYS(chip, opcode);
            break;
        }

        break;

    case 0x1:
        JMP(chip, opcode);
        break;

    case 0x2:
        CALL(chip, opcode);
        break;

    case 0x3:
        SE(chip, opcode);
        break;

    case 0x4:
        SNE(chip, opcode);
        break;

    case 0x5:
        if (N0(opcode) == 0x0)
        {
            SE(chip, opcode);
        }
        else
        {
            printf("Invalid opcode: 0x%4X\n", opcode);
            chip->halt = true;
        }

        break;

    case 0x6:
        LD(chip, opcode);
        break;

    case 0x7:
        ADD(chip, opcode);
        break;

    case 0x8:
        switch (opcode & 0xF)
        {
        case 0x0:
            LD(chip, opcode);
            break;

        case 0x1:
            OR(chip, opcode);
            break;

        case 0x2:
            AND(chip, opcode);
            break;

        case 0x3:
            XOR(chip, opcode);
            break;

        case 0x4:
            ADD(chip, opcode);
            break;

        case 0x5:
            SUB(chip, opcode);
            break;

        case 0x6:
            SHR(chip, opcode);
            break;

        case 0x7:
            SUBN(chip, opcode);
            break;

        case 0xE:
            SHL(chip, opcode);
            break;

        default:
            printf("Invalid opcode: 0x%4X\n", opcode);
            chip->halt = true;
            break;
        }

        break;

    case 0x9:
        if (N0(opcode) == 0)
        {
            SNE(chip, opcode);
        }
        else
        {
            printf("Invalid opcode: 0x%4X\n", opcode);
            chip->halt = true;
        }

        break;

    case 0xA:
        LD(chip, opcode);
        break;

    case 0xB:
        JMP(chip, opcode);
        break;

    case 0xC:
        RND(chip, opcode);
        break;

    case 0xD:
        DRW(chip, opcode);
        break;

    case 0xE:
        switch (opcode & 0xFF)
        {
        case 0x9E:
            SKP(chip, opcode);
            break;

        case 0xA1:
            SKNP(chip, opcode);
            break;

        default:
            printf("Invalid opcode: 0x%4X\n", opcode);
            chip->halt = true;
            break;
        }

        break;

    case 0xF:
        switch (opcode & 0xFF)
        {
        case 0x7:
            LD(chip, opcode);
            break;

        case 0xA:
            LD(chip, opcode);
            break;

        case 0x15:
            LD(chip, opcode);
            break;

        case 0x18:
            LD(chip, opcode);
            break;

        case 0x1E:
            ADD(chip, opcode);
            break;

        case 0x29:
            LD(chip, opcode);
            break;

        case 0x33:
            LD(chip, opcode);
            break;

        case 0x55:
            LD(chip, opcode);
            break;

        case 0x65:
            LD(chip, opcode);
            break;

        default:
            printf("Invalid opcode: 0x%4X\n", opcode);
            chip->halt = true;
            break;
        }

        break;

    default:
        printf("Invalid opcode: 0x%4X\n", opcode);
        chip->halt = true;
        break;
    }
}


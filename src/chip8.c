#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"

#define N0(x) ((x) & 0xF)
#define N1(x) (((x) >> 4) & 0xF)
#define N2(x) (((x) >> 8) & 0xF)
#define N3(x) (((x) >> 12) & 0xF)

static const uint8_t fontset[80] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80    // F
};

static
void
ResetMemory(CHIP8 *chip)
{
    memset(chip->Memory, 0, sizeof(chip->Memory));

    for (int i = 0; i < 80; i++)
        chip->Memory[i] = fontset[i];
}

void
Reset(CHIP8 *chip)
{
    chip->I = 0;
    chip->DelayTimer = 0;
    chip->SoundTimer = 0;
    chip->PC = 0x200;
    chip->SP = -1;

    memset(chip->V, 0, sizeof(chip->V));
    memset(chip->Stack, 0, sizeof(chip->Stack));
    memset(chip->Display, 0, sizeof(chip->Display));
    memset(chip->Keypad, 0, sizeof(chip->Keypad));

    chip->Draw = false;
    chip->Halt = false;
}

bool
LoadROM(CHIP8 *chip, const char *rom)
{
    FILE *fp;
    long size;

    fp = fopen(rom, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", rom);
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    if (size > (0x1000 - 0x200))
    {
        fprintf(stderr, "ROM too big\n");
        fclose(fp);
        return false;
    }

    ResetMemory(chip);

    for (int i = 0; i < size; i++)
    {
        chip->Memory[i + 0x200] = (uint8_t)fgetc(fp);
    }

    fclose(fp);
    return true;
}

void
UpdateTimers(CHIP8 *chip)
{
    if (chip->DelayTimer > 0)
        (chip->DelayTimer)--;

    if (chip->SoundTimer > 0)
        (chip->SoundTimer)--;
}

void
FDX(CHIP8 *chip)
{
    uint16_t opcode = (chip->Memory[chip->PC] << 8) | chip->Memory[chip->PC + 1];
    chip->PC += 2;

    switch (N3(opcode))
    {
    case 0x0:
        switch (opcode & 0xFFF)
        {
        case 0x0E0:
            /* 00E0: Clears the screen. */
            memset(chip->Display, 0, sizeof(chip->Display));
            chip->Draw = true;
            break;

        case 0x0EE:
            /* 00EE: Returns from a subroutine. */
            chip->PC = chip->Stack[(chip->SP)--];
            break;

        default:
            /* 0NNN: Calls RCA 1802 program at address NNN. */
            /* This instruction is ignored by modern interpreters. */
            chip->Halt = true;
            break;
        }

        break;

    case 0x1:
        /* 1NNN: Jump to address NNN. */
        chip->PC = opcode & 0xFFF;
        break;

    case 0x2:
        /* 2NNN: Calls subroutine at NNN. */
        chip->Stack[++(chip->SP)] = chip->PC;
        chip->PC = opcode & 0xFFF;
        break;

    case 0x3:
        /* 3XNN: Skips the next instruction if VX is equal to NN. */
        if (chip->V[N2(opcode)] == (opcode & 0xFF))
        {
            chip->PC += 2;
        }

        break;

    case 0x4:
        /* 4XNN: Skips the next instruction if VX is not equal to NN. */
        if (chip->V[N2(opcode)] != (opcode & 0xFF))
        {
            chip->PC += 2;
        }

        break;

    case 0x5:
        if (N0(opcode) == 0x0)
        {
            /* 5XY0: Skips the next instruction if VX is equal to VY. */
            if (chip->V[N2(opcode)] == chip->V[N1(opcode)])
            {
                chip->PC += 2;
            }
        }
        else
        {
            fprintf(stderr, "Invalid opcode: 0x%04X\n", opcode);
            chip->Halt = true;
        }

        break;

    case 0x6:
        /* 6XNN: Sets VX to NN. */
        chip->V[N2(opcode)] = opcode & 0xFF;
        break;

    case 0x7:
        /* 7XNN: Adds NN to VX. */
        chip->V[N2(opcode)] += opcode & 0xFF;
        break;

    case 0x8:
        switch (opcode & 0xF)
        {
        case 0x0:
            /* 8XY0: Sets VX to the value of VY. */
            chip->V[N2(opcode)] = chip->V[N1(opcode)];
            break;

        case 0x1:
            /* 8XY1: Sets VX to (VX | VY). */
            chip->V[N2(opcode)] |= chip->V[N1(opcode)];
            break;

        case 0x2:
            /* 8XY2: Sets VX to (VX & VY). */
            chip->V[N2(opcode)] &= chip->V[N1(opcode)];
            break;

        case 0x3:
            /* 8XY3: Sets VX to (VX ^ VY). */
            chip->V[N2(opcode)] ^= chip->V[N1(opcode)];
            break;

        case 0x4:
            /* 8XY4: Adds VY to VX. VF is set to 1 when there is a carry,
             *       and to 0 when there is not.
             */
            if ((chip->V[N2(opcode)] + chip->V[N1(opcode)]) > 0xFF)
            {
                chip->V[0xF] = 1;
            }
            else
            {
                chip->V[0xF] = 0;
            }

            chip->V[N2(opcode)] += chip->V[N1(opcode)];
            break;

        case 0x5:
            /* 8XY5: VY is subtracted from VX. VF is set to 0 when there
             *       is a borrow, and to 1 when there is not.
             */
            if (chip->V[N2(opcode)] < chip->V[N1(opcode)])
            {
                chip->V[0xF] = 0;
            }
            else
            {
                chip->V[0xF] = 1;
            }

            chip->V[N2(opcode)] -= chip->V[N1(opcode)];
            break;

        case 0x6:
            /* 8XY6: Shifts VX right by one. VF is set to the value of
             *       the least significant bit of VX before the shift.
             */
            chip->V[0xF] = chip->V[N2(opcode)] & 0x1;
            chip->V[N2(opcode)] = chip->V[N2(opcode)] >> 1;
            break;

        case 0x7:
            /* 8XY7: Sets VX to VY minus VX. VF is set to 0 when there
             *       is a borrow, and to 1 when there is not.
             */
            if (chip->V[N2(opcode)] > chip->V[N1(opcode)])
            {
                chip->V[0xF] = 0;
            }
            else
            {
                chip->V[0xF] = 1;
            }

            chip->V[N2(opcode)] = chip->V[N1(opcode)] - chip->V[N2(opcode)];
            break;

        case 0xE:
            /* 8XYE: Shifts VX left by one. VF is set to the value of
             *       the most significant bit of VX before the shift.
             */
            chip->V[0xF] = (chip->V[N2(opcode)] >> 7) & 0x1;
            chip->V[N2(opcode)] = chip->V[N2(opcode)] << 1;
            break;

        default:
            fprintf(stderr, "Invalid opcode: 0x%04X\n", opcode);
            chip->Halt = true;
            break;
        }

        break;

    case 0x9:
        if (N0(opcode) == 0)
        {
            /* 9XY0: Skips the next instruction of VX is not equal to VY. */
            if (chip->V[N2(opcode)] != chip->V[N1(opcode)])
                chip->PC += 2;
        }
        else
        {
            fprintf(stderr, "Invalid opcode: 0x%04X\n", opcode);
            chip->Halt = true;
        }

        break;

    case 0xA:
        /* ANNN: Sets I to the address NNN. */
        chip->I = opcode & 0xFFF;
        break;

    case 0xB:
        /* BNNN: Jumps to the address NNN plus V0. */
        chip->PC = (opcode & 0xFFF) + chip->V[0];
        break;

    case 0xC:
        /* CXNN: Sets VX to a random number AND NN. */
        chip->V[N2(opcode)] = (opcode & 0xFF) & (rand() & 0xFF);
        break;

    case 0xD:
        /* DXYN: Draw a sprite at (VX, VY) with N bytes of sprite data
         *       starting at the address stored in I. Set VF to 1 if a
         *       pixel is unset and to 0 otherwise.
         */

        {
            uint8_t height = N0(opcode);

            /* Reset VF before checking for collision. */
            chip->V[0xF] = 0;

            for (int row = 0; row < height; row++)
            {
                uint8_t sprite_row = chip->Memory[chip->I + row];

                /* Scan through the byte to check if each bit is set. */
                for (int col = 0; col < 8; col++)
                {
                    /* We only care if the pixel of the sprite is set. */
                    if ((sprite_row & (0x80 >> col)) != 0)
                    {
                        /* Allow wrapping on the display. */
                        uint8_t x = (chip->V[N2(opcode)] + col) % 64;
                        uint8_t y = (chip->V[N1(opcode)] + row) % 32;

                        int pos = (y * 64) + x;

                        /* Check for collision. */
                        if (chip->Display[pos] == 1)
                            chip->V[0xF] = 1;

                        chip->Display[pos] ^= 1;
                    }
                }
            }
        }

        chip->Draw = true;
        break;

    case 0xE:
        switch (opcode & 0xFF)
        {
        case 0x9E:
            /* EX9E: Skips the next instruction of the key stored in VX is pressed. */
            if (chip->Keypad[chip->V[N2(opcode)]])
                chip->PC += 2;

            break;

        case 0xA1:
            /* EXA1: Skips the next instruction of the key stored in VX is not pressed. */
            if (!chip->Keypad[chip->V[N2(opcode)]])
                chip->PC += 2;

            break;

        default:
            fprintf(stderr, "Invalid opcode: 0x%04X\n", opcode);
            chip->Halt = true;
            break;
        }

        break;

    case 0xF:
        switch (opcode & 0xFF)
        {
        case 0x7:
            /* FX07: Sets VX to the value of the delay timer. */
            chip->V[N2(opcode)] = chip->DelayTimer;
            break;

        case 0xA:
            /* FX0A: A key press is awaited, then stored in VX. */
            {
                bool pressed = false;

                for (int i = 0; i < 16; i++)
                {
                    if (chip->Keypad[i])
                    {
                        pressed = true;
                        break;
                    }
                }

                if (!pressed)
                {
                    chip->PC -= 2;
                }
            }

            break;

        case 0x15:
            /* FX15: Sets the delay timer to VX. */
            chip->DelayTimer = chip->V[N2(opcode)];
            break;

        case 0x18:
            /* FX18: Sets the sound timer to VX. */
            chip->SoundTimer = chip->V[N2(opcode)];
            break;

        case 0x1E:
            /* FX1E: Adds VX to I. */
            if ((chip->I + chip->V[N2(opcode)]) > 0xFFF)
            {
                chip->V[0xF] = 1;
            }
            else
            {
                chip->V[0xF] = 0;
            }

            chip->I += chip->V[N2(opcode)];
            break;

        case 0x29:
            /* FX29: Sets I to the location of the sprite for the character in VX. */
            chip->I = chip->V[N2(opcode)] * 5;
            break;

        case 0x33:
            /* FX33: Stores the BCD representation of VX. */
            {
                uint8_t value = chip->V[N2(opcode)];

                chip->Memory[chip->I] = value / 100;
                chip->Memory[chip->I + 1] = (value / 10) % 10;
                chip->Memory[chip->I + 2] = value % 10;
            }

            break;

        case 0x55:
            /* FX55: Stores V0 to VX in memory starting at address I. */
            for (int i = 0; i <= N2(opcode); i++)
            {
                chip->Memory[chip->I + i] = chip->V[i];
            }

            break;

        case 0x65:
            /* FX65: Fills V0 to VX with values from memory starting at address I. */
            for (int i = 0; i <= N2(opcode); i++)
            {
                chip->V[i] = chip->Memory[chip->I + i];
            }

            break;

        default:
            fprintf(stderr, "Invalid opcode: 0x%04X\n", opcode);
            chip->Halt = true;
            break;
        }

        break;

    default:
        fprintf(stderr, "Invalid opcode: 0x%04X\n", opcode);
        chip->Halt = true;
        break;
    }
}


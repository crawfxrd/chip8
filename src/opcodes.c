#include "opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ADD(Chip8 *chip, uint16_t opcode)
{
    if (N3(opcode) == 0x7)
    {
        /* 7NXX: Adds NN to VX. */

        chip->V[N2(opcode)] += opcode & 0xFF;
    }
    else if (N3(opcode) == 0x8)
    {
        /* 8XY4: Adds VY to VX. VF is set to 1 when there is a carry, and
         *       to 0 when there is not.
         */

        if (chip->V[N2(opcode)] + chip->V[N1(opcode)] > 0xFF)
            chip->V[0xF] = 1;
        else
            chip->V[0xF] = 0;

        chip->V[N2(opcode)] += chip->V[N1(opcode)];
    }
    else if (N3(opcode) == 0xF)
    {
        /* FX1E: Adds VX to I. */

        if (chip->I + chip->V[N2(opcode)] > 0xFFF)
            chip->V[0xF] = 1;
        else
            chip->V[0xF] = 0;

        chip->I += chip->V[N2(opcode)];
    }

    chip->pc += 2;
}

void AND(Chip8 *chip, uint16_t opcode)
{
    /* 8XY2: Sets VX to VX AND VY. */

    chip->V[N2(opcode)] &= chip->V[N1(opcode)];

    chip->pc += 2;
}

void CALL(Chip8 *chip, uint16_t opcode)
{
    /* 2NNN: Calls subroutine at NNN. */

    /* Push the address of the next instruction to be executed. */
    chip->stack[++(chip->sp)] = chip->pc + 2;

    chip->pc = opcode & 0xFFF;
}

void CLS(Chip8 *chip)
{
    /* 00E0: Clears the screen. */

    memset(chip->graphics, 0, sizeof(chip->graphics));
    chip->draw_flag = true;

    chip->pc += 2;
}

void DRW(Chip8 *chip, uint16_t opcode)
{
    /* DXYN: Draw a sprite at (VX, VY) with N bytes of sprite data starting
     *       at the address stored in I. Set VF to 1 if a pixel is unset
     *       and 0 otherwise.
     */

    uint8_t x = chip->V[N2(opcode)];
    uint8_t y = chip->V[N1(opcode)];
    uint8_t height = N0(opcode);

    /* Reset VF before checking for collision. */
    chip->V[0xF] = 0;

    for (int row = 0; row < height; row++)
    {
        uint8_t sprite_row = chip->memory[chip->I + row];

        /* Scan through the byte to check if each bit is set. */
        for (int col = 0; col < 8; col++)
        {
            /* We onlt care if the pixel of the sprite is set. */
            if ((sprite_row & (0x80 >> col)) != 0)
            {
                int pos = ((y + row) * 64) + (x + col);

                /* Check for collision. */
                if (chip->graphics[pos] == 1)
                    chip->V[0xF] = 1;

                chip->graphics[pos] ^= 1;
            }
        }
    }

    chip->draw_flag = true;

    chip->pc += 2;
}

void JMP(Chip8 *chip, uint16_t opcode)
{
    if (N3(opcode) == 0x1)
    {
        /* 1NNN: Jump to address NNN. */

        chip->pc = opcode & 0xFFF;
    }
    else if (N3(opcode) == 0xB)
    {
        /* BNNN: Jumps to the address NNN plus V0. */

        chip->pc = (opcode & 0xFFF) + chip->V[0];
    }
}

void LD(Chip8 *chip, uint16_t opcode)
{
    if (N3(opcode) == 0x6)
    {
        /* 6XNN: Sets VX to NN. */

        chip->V[N2(opcode)] = opcode & 0xFF;
    }
    else if (N3(opcode) == 0x8)
    {
        /* 8XY0: Sets VX to the value of VY. */

        chip->V[N2(opcode)] = chip->V[N1(opcode)];
    }
    else if (N3(opcode) == 0xA)
    {
        /* ANNN: Sets I to the address NNN. */

        chip->I = opcode & 0xFFF;
    }
    else if (N3(opcode) == 0xF)
    {
        if ((opcode & 0xFF) == 0x7)
        {
            /* FX07: Sets VX to the value of the delay timer. */

            chip->V[N2(opcode)] = chip->delay_timer;
        }
        else if ((opcode & 0xFF) == 0xA)
        {
            /* FX0A: A key press is awaited, and then stored in VX. */

            chip->V[N2(opcode)] = chip8_getkey(chip);
        }
        else if ((opcode & 0xFF) == 0x15)
        {
            /* FX15: Sets the delay timer to VX. */

            chip->delay_timer = chip->V[N2(opcode)];
        }
        else if ((opcode & 0xFF) == 0x18)
        {
            /* FX18: Sets the sound timer to VX. */

            chip->sound_timer = chip->V[N2(opcode)];
        }
        else if ((opcode & 0xFF) == 0x29)
        {
            /* FX29: Sets I to the location of the sprite for the character
             *       in VX. Characters 0-F are represented by a 4x5 font.
             */

            chip->I = chip->V[N2(opcode)] * 5;
        }
        else if ((opcode & 0xFF) == 0x33)
        {
            /* FX33: Stores the binary-coded decimal representation of VX,
             *       with the most significant digit at the address in I, the
             *       middle digit at I+1, and the least significant at I+2.
             */

            uint8_t value = chip->V[N2(opcode)];

            chip->memory[chip->I] = value / 100;
            chip->memory[chip->I + 1] = (value / 10) % 10;
            chip->memory[chip->I + 2] = value % 10;
        }
        else if ((opcode & 0xFF) == 0x55)
        {
            /* FX55: Stores V0 to VX in memory starting at address I. */

            for (int i = 0; i <= N2(opcode); i++)
                chip->memory[chip->I + i] = chip->V[i];
        }
        else if ((opcode & 0xFF) == 0x65)
        {
            /* FX65: Fills V0 to VX with values from memory starting at address I. */

            for (int i = 0; i <= N2(opcode); i++)
                chip->V[i] = chip->memory[chip->I + i];
        }
    }

    chip->pc += 2;
}

void OR(Chip8 *chip, uint16_t opcode)
{
    /* 8XY1: Sets VX to VX OR VY. */

    chip->V[N2(opcode)] |= chip->V[N1(opcode)];

    chip->pc += 2;
}

void RET(Chip8 *chip)
{
    /* 00EE: Returns from a subroutine. */

    chip->pc = chip->stack[(chip->sp)--];
}

void RND(Chip8 *chip, uint16_t opcode)
{
    /* CXNN: Sets VX to a random number AND NN. */

    chip->V[N2(opcode)] = (opcode & 0xFF) & (rand() & 0xFF);

    chip->pc += 2;
}

void SE(Chip8 *chip, uint16_t opcode)
{
    if (N3(opcode) == 0x3)
    {
        /* 3XNN: Skips the next instruction if VX equals NN. */

        if (chip->V[N2(opcode)] == (opcode & 0xFF))
            chip->pc += 2;
    }
    else if (N3(opcode) == 0x5)
    {
        /* 5XY0: Skips the next instruction if VX equals VY. */

        if (chip->V[N2(opcode)] == chip->V[N1(opcode)])
            chip->pc += 2;
    }

    chip->pc += 2;
}

void SHL(Chip8 *chip, uint16_t opcode)
{
    /* 8XYE: Shifts VX left by one. VF is set to the value of the
     *       most significant bit of VX before the shift.
     */

    chip->V[0xF] = (chip->V[N2(opcode)] >> 7) & 0x1;
    chip->V[N2(opcode)] = chip->V[N2(opcode)] << 1;

    chip->pc += 2;
}

void SHR(Chip8 *chip, uint16_t opcode)
{
    /* 8XY6: Shifts VX right by one. VF is set to the value of the
     *       least significant bit of VX before the shift.
     */

    chip->V[0xF] = chip->V[N2(opcode)] & 0x1;
    chip->V[N2(opcode)] = chip->V[N2(opcode)] >> 1;

    chip->pc += 2;
}

void SKNP(Chip8 *chip, uint16_t opcode)
{
    /* EXA1: Skips the next instruction if the key stored in VX is not pressed. */

    if (!chip->keypad[chip->V[N2(opcode)]])
        chip->pc += 2;

    chip->pc += 2;
}

void SKP(Chip8 *chip, uint16_t opcode)
{
    /* EX9E: Skips the next instruction if the key stored in VX is pressed. */

    if (chip->keypad[chip->V[N2(opcode)]])
        chip->pc += 2;

    chip->pc += 2;
}

void SNE(Chip8 *chip, uint16_t opcode)
{
    if (N3(opcode) == 0x4)
    {
        /* 4XNN: Skips the next instruction if VX does not equal NN. */

        if (chip->V[N2(opcode)] != (opcode & 0xFF))
            chip->pc += 2;
    }
    else if (N3(opcode) == 0x9 && N0(opcode) == 0)
    {
        /* 9XY0: Skips the next instruction if VX does not equal VY. */

        if (chip->V[N2(opcode)] != chip->V[N1(opcode)])
            chip->pc += 2;
    }

    chip->pc += 2;
}

void SUB(Chip8 *chip, uint16_t opcode)
{
    /* 8XY5: VY is subtracted from VX. VF is set to 0 when there is
     *       a borrow, and 1 when there is not.
     */

    if (chip->V[N2(opcode)] < chip->V[N1(opcode)])
        chip->V[0xF] = 0;
    else
        chip->V[0xF] = 1;

    chip->V[N2(opcode)] -= chip->V[N1(opcode)];

    chip->pc += 2;
}

void SUBN(Chip8 *chip, uint16_t opcode)
{
    /* 8XY7: Sets VX to VY minus VX. VF is set to 0 when there is
     *       a borrow, and 1 when there is not.
     */

    if (chip->V[N2(opcode)] > chip->V[N1(opcode)])
        chip->V[0xF] = 0;
    else
        chip->V[0xF] = 1;

    chip->V[N2(opcode)] = chip->V[N1(opcode)] - chip->V[N2(opcode)];

    chip->pc += 2;
}

/* This instruction is ignored by modern interpreters. */
void SYS(Chip8 *chip, uint16_t opcode)
{
    /* 0NNN: Calls RCA 1802 program at address NNN. */

    chip->halt = true;
}

void XOR(Chip8 *chip, uint16_t opcode)
{
    /* 8XY3: Sets VX to VX XOR VY. */

    chip->V[N2(opcode)] ^= chip->V[N1(opcode)];

    chip->pc += 2;
}


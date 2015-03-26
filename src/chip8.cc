#include "chip8.h"
#include "graphics.h"
#include "input.h"

#include <algorithm>
#include <cstring>
#include <SDL2/SDL.h>


Chip8::Chip8() :
    memory_{{0}},
    graphics_map_{{0}},
    V_{{0}},
    stack_{{0}},
    sp_(-1),
    I_(0),
    pc_(0x200),
    delay_timer_(0),
    sound_timer_(0),
    redraw_(false),
    halt_(false),
    mt_{std::random_device{}()}
{
    SDL_Init(SDL_INIT_EVERYTHING);
    LoadFontSet();
}

Chip8::~Chip8()
{
    SDL_Quit();
}

void Chip8::LoadFontSet()
{
    const uint8_t fontset[80] =
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
        0xF0, 0x80, 0xF0, 0x80, 0x80    // F
    };

    for (int i = 0; i < 80; i++)
        memory_[i] = fontset[i];
}

bool Chip8::LoadRom(const std::string& rom)
{
    FILE* fp;
    long size;

    fp = fopen(rom.c_str(), "rb");
    if (fp == nullptr)
    {
        std::cout << "Unable to open file " << rom << std::endl;
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    if (size > (0x1000 - 0x200))
    {
        std::cout << "ROM too big" << std::endl;
        fclose(fp);
        return false;
    }

    for (int i = 0; i < size; i++)
        memory_[i + 0x200] = static_cast<uint8_t>(fgetc(fp));

    fclose(fp);
    return true;
}

void Chip8::Run()
{
    Graphics graphics;
    Input input;
    SDL_Event event;

    while (!halt_)
    {
        Cycle(input);

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    return;

                case SDL_KEYDOWN:
                    input.KeyDownEvent(event);
                    break;

                case SDL_KEYUP:
                    input.KeyUpEvent(event);
                    break;
            }
        }

        if (input.KeyPressed(SDLK_ESCAPE))
            halt_ = true;

        if (redraw_)
            Draw(graphics);

        if (delay_timer_ > 0)
            delay_timer_--;

        if (sound_timer_ > 0)
            sound_timer_--;

        SDL_Delay(2);
    }
}

void Chip8::Draw(Graphics& graphics)
{
    graphics.Clear();
    graphics.FillRects(graphics_map_);
    graphics.Render();
}

uint8_t Chip8::RandomByte()
{
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    return dist(mt_);
}

void Chip8::Cycle(Input& input)
{
    uint16_t opcode = (memory_[pc_] << 8) | memory_[pc_ + 1];

    uint8_t X = (opcode >> 8) & 0xF;
    uint8_t Y = (opcode >> 4) & 0xF;

    switch ((opcode >> 12) & 0xF)
    {
    case 0x0:
        switch (opcode & 0xFFF)
        {
        case 0x0E0:
            /* 00E0: Clears the screen. */
            graphics_map_.fill(0);
            redraw_ = true;
            pc_ += 2;
            break;

        case 0x0EE:
            /* 00EE: Returns from a subroutine. */
            pc_ = stack_[sp_--];
            break;

        default:
            /* 0NNN: Calls RCA 1802 program at address NNN. */
            /* This instruction is ignored by modern interpreters. */
            printf("Invalid opcode: 0x%4X\n", opcode);
            halt_ = true;
            break;
        }

        break;

    case 0x1:
        /* 1NNN: Jumps to address NNN. */
        pc_ = opcode & 0xFFF;
        break;

    case 0x2:
        /* 2NNN: Calls the subroutine at address NNN. */
        stack_[++sp_] = pc_ + 2;
        pc_ = opcode & 0xFFF;
        break;

    case 0x3:
        /* 3XNN: Skips the next instruction if VX equals NN. */
        if (V_[X] == (opcode & 0xFF))
            pc_ += 2;

        pc_ += 2;
        break;

    case 0x4:
        /* 4XNN: Skips the next instruction if VX does not equal NN. */
        if (V_[X] != (opcode & 0xFF))
            pc_ += 2;

        pc_ += 2;
        break;

    case 0x5:
        if ((opcode & 0xF) == 0x0)
        {
            /* 5XY0: Skips the next instruction if VX equals VY. */
            if (V_[X] == V_[Y])
                pc_ += 2;

            pc_ += 2;
        }
        else
        {
            printf("Invalid opcode: 0x%4X\n", opcode);
            halt_ = true;
        }

        break;

    case 0x6:
        /* 6XNN: Sets VX to NN. */
        V_[X] = opcode & 0xFF;
        pc_ += 2;
        break;

    case 0x7:
        /* 7XNN: Adds NN to VX. */
        V_[X] += opcode & 0xFF;
        pc_ += 2;
        break;

    case 0x8:
        switch (opcode & 0xF)
        {
        case 0x0:
            /* 8XY0: Sets VX to the value of VY. */
            V_[X] = V_[Y];
            pc_ += 2;
            break;

        case 0x1:
            /* 8XY1: Sets VX to VX OR VY. */
            V_[X] |= V_[Y];
            pc_ += 2;
            break;

        case 0x2:
            /* 8XY2: Sets VX to VX AND VY. */
            V_[X] &= V_[Y];
            pc_ += 2;
            break;

        case 0x3:
            /* 8XY3: Sets VX to VX XOR VY. */
            V_[X] ^= V_[Y];
            pc_ += 2;
            break;

        case 0x4:
            /* 8XY4: Adds VY to VX. VF is set to 1 when there is a carry, and
             *       to 0 when there is not.
             */
            if ((V_[X] + V_[Y]) > 0xFF)
                V_[0xF] = 1;
            else
                V_[0xF] = 0;

            V_[X] += V_[Y];
            pc_ += 2;
            break;

        case 0x5:
            /* 8XY5: Subtracts VY from VX. VF is set to 0 when there is a
             *       borrow, and to 1 when there is not.
             */
            if (V_[X] < V_[Y])
                V_[0xF] = 0;
            else
                V_[0xF] = 1;

            V_[X] -= V_[Y];
            pc_ += 2;
            break;

        case 0x6:
            /* 8XY6: Shifts VX right by one. VF is set to the value of the
             *       LSB of VX before the shift.
             */
            V_[0xF] = V_[X] & 0x1;
            V_[X] >>= 1;
            pc_ += 2;
            break;

        case 0x7:
            /* 8XY7: Sets VX to VY minus VX. VF is set to 0 when there is a
             *       borrow, and to 1 when there is not.
             */
            if (V_[X] > V_[Y])
                V_[0xF] = 0;
            else
                V_[0xF] = 1;

            V_[X] = V_[Y] - V_[X];
            pc_ += 2;
            break;

        case 0xE:
            /* 8XYE: Shift VX left by one. VF is set to the value of the
             *       MSB of VX before the shift.
             */
            V_[0xF] = (V_[X] >> 7) & 0x1;
            V_[X] <<= 1;
            pc_ += 2;
            break;

        default:
            printf("Invalid opcode: 0x%4X\n", opcode);
            halt_ = true;
            break;
        }

        break;

    case 0x9:
        if ((opcode & 0xF) == 0)
        {
            /* 9XY0: Skips the next instruction if VX does not equal VY. */
            if (V_[X] != V_[Y])
                pc_ += 2;

            pc_ += 2;
        }
        else
        {
            printf("Invalid opcode: 0x%4X\n", opcode);
            halt_ = true;
        }

        break;

    case 0xA:
        /* ANNN: Sets I to the address NNN. */
        I_ = opcode & 0xFFF;
        pc_ += 2;
        break;

    case 0xB:
        /* BNNN: Jumps to the address NNN plus V0. */
        pc_ = (opcode & 0xFFF) + V_[0];
        break;

    case 0xC:
        /* CXNN: Sets VX to a random number AND NN. */
        V_[X] = (opcode & 0xFF) & RandomByte();
        pc_ += 2;
        break;

    case 0xD:
        /* DXYN: Draws a sprite at (VX, VY) with N bytes of sprite data
         *       starting at the address stored in I. Sets VF to 1 if a pixel
         *       is unset and to 0 otherwise.
         */

        /* Reset VF before checking for collision. */
        V_[0xF] = 0;

        for (int row = 0; row < (opcode & 0xF); row++)
        {
            uint8_t sprite_row = memory_[I_ + row];

            /* Scan through the byte to check if each bit is set. */
            for (int col = 0; col < 8; col++)
            {
                /* We only care if the pixel of the sprite is set. */
                if ((sprite_row & (0x80 >> col)) != 0)
                {
                    int pos = ((V_[Y] + row) * 64) + (V_[X] + col);

                    /* Check collision. */
                    if (graphics_map_[pos] == 1)
                        V_[0xF] = 1;

                    graphics_map_[pos] ^= 1;
                }
            }
        }

        redraw_ = true;
        pc_ += 2;
        break;

    case 0xE:
        switch (opcode & 0xFF)
        {
        case 0x9E:
            /* EX9E: Skips the next instruction if the key stored in VX is pressed. */
            if (input.KeyPressed(input.Translate(V_[X])))
                pc_ += 2;

            pc_ += 2;
            break;

        case 0xA1:
            /* EXA1: Skips the next instruction if the key stored in VX is not pressed. */
            if (!input.KeyPressed(input.Translate(V_[X])))
                pc_ += 2;

            pc_ += 2;
            break;

        default:
            printf("Invalid opcode: 0x%4X\n", opcode);
            halt_ = true;
            break;
        }

        break;

    case 0xF:
        switch (opcode & 0xFF)
        {
        case 0x7:
            /* FX07: Sets VX to the value of the delay timer. */
            V_[X] = delay_timer_;
            pc_ += 2;
            break;

        case 0xA:
            /* FX0A: A key press is awaited, then stored in VX. */
            V_[X] = input.WaitKey();
            pc_ += 2;
            break;

        case 0x15:
            /* FX15: Sets the delay timer to VX. */
            delay_timer_ = V_[X];
            pc_ += 2;
            break;

        case 0x18:
            /* FX18: Sets the sound timer to VX. */
            sound_timer_ = V_[X];
            pc_ += 2;
            break;

        case 0x1E:
            /* FX1E: Adds VX to I. */
            if ((I_ + V_[X]) > 0xFFF)
                V_[0xF] = 1;
            else
                V_[0xF] = 0;

            I_ += V_[X];
            pc_ += 2;
            break;

        case 0x29:
            /* FX29: Sets I to the location of the sprite for the character in VX. */
            I_ = V_[X] * 5;
            pc_ += 2;
            break;

        case 0x33:
            /* FX33: Stores the BCD representation of VX, with the most
             *       significant digit at address I, the middle digit at I+1,
             *       and the least significant digit at I+2.
             */
            memory_[I_] = V_[X] / 100;
            memory_[I_ + 1] = (V_[X] / 10) % 10;
            memory_[I_ + 2] = V_[X] % 10;

            pc_ += 2;
            break;

        case 0x55:
            /* FX55: Stores V0 to VX in memory starting at address I. */
            for (int i = 0; i <= X; i++)
                memory_[I_ + i] = V_[i];

            pc_ += 2;
            break;

        case 0x65:
            /* FX65: Fills V0 to VX with values from memory starting at address I. */
            for (int i = 0; i <= X; i++)
                V_[i] = memory_[I_ + i];

            pc_ += 2;
            break;

        default:
            printf("Invalid opcode: 0x%4X\n", opcode);
            halt_ = true;
            break;
        }

        break;

    default:
        printf("Invalid opcode: 0x%4X\n", opcode);
        halt_ = true;
        break;
    }
}


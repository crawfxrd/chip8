#include "chip8.h"
#include "opcodes.h"
#include "sdl.h"
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
    0xF0, 0x80, 0xF0, 0x80, 0x80    // F
};

static void Cycle(Chip8 *chip);
static void HandleIRQ(Chip8 *chip);
static void UpdateTimers(Chip8 *chip);
static void SetKey(Chip8 *chip, bool isdown);

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
    memset(chip->keypad, false, sizeof(chip->keypad));

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
    if (!sdl_init())
    {
        printf("Error starting SDL\n");
        chip->halt = true;
    }

    while(!(chip->halt))
    {
        Cycle(chip);

        while (SDL_PollEvent(&chip->event))
            HandleIRQ(chip);

        UpdateTimers(chip);

        if (chip->draw_flag)
            sdl_render(chip);

        SDL_Delay(2);
    }

    sdl_cleanup();
}

uint8_t chip8_getkey(Chip8 *chip)
{
    while (true)
    {
        SDL_WaitEvent(&chip->event);

        if (chip->event.type != SDL_KEYDOWN)
            continue;

        SetKey(chip, true);
        switch (chip->event.key.keysym.sym)
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

        case SDLK_SPACE:
            chip->halt = true;
            return 0xFF;
        }
    }
}

static void HandleIRQ(Chip8 *chip)
{
    switch (chip->event.type)
    {
    case SDL_QUIT:
        chip->halt = true;
        break;

    case SDL_KEYDOWN:
        if (chip->event.key.keysym.sym == SDLK_ESCAPE)
            chip->halt = true;
        else
            SetKey(chip, true);

        break;

    case SDL_KEYUP:
        SetKey(chip, false);
        break;
    }
}

static void SetKey(Chip8 *chip, bool isdown)
{
    switch (chip->event.key.keysym.sym)
    {
    case SDLK_1: chip->keypad[0x1] = isdown; break;
    case SDLK_2: chip->keypad[0x2] = isdown; break;
    case SDLK_3: chip->keypad[0x3] = isdown; break;
    case SDLK_4: chip->keypad[0xC] = isdown; break;

    case SDLK_q: chip->keypad[0x4] = isdown; break;
    case SDLK_w: chip->keypad[0x5] = isdown; break;
    case SDLK_e: chip->keypad[0x6] = isdown; break;
    case SDLK_r: chip->keypad[0xD] = isdown; break;

    case SDLK_a: chip->keypad[0x7] = isdown; break;
    case SDLK_s: chip->keypad[0x8] = isdown; break;
    case SDLK_d: chip->keypad[0x9] = isdown; break;
    case SDLK_f: chip->keypad[0xE] = isdown; break;

    case SDLK_z: chip->keypad[0xA] = isdown; break;
    case SDLK_x: chip->keypad[0x0] = isdown; break;
    case SDLK_c: chip->keypad[0xB] = isdown; break;
    case SDLK_v: chip->keypad[0xF] = isdown; break;
    }
}

static void UpdateTimers(Chip8 *chip)
{
    if (chip->delay_timer > 0)
        (chip->delay_timer)--;

    if (chip->sound_timer > 0)
        (chip->sound_timer)--;
}

static void Cycle(Chip8 *chip)
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


#include <stdio.h>
#include "chip8.h"

int main(int argc, char *argv[])
{
    Chip8 chip;

    if (argc != 2)
    {
        puts("Usage: chip8 rom");
        return 0;
    }

    chip8_init(&chip);
    chip8_dump(&chip);

    if (chip8_load(&chip, argv[1]) == 0)
    {
        puts("ROM read into memory.");
    }
    else
    {
        puts("Failed to load ROM.");
    }

    return 0;
}


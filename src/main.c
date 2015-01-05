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

    if (!chip8_load(&chip, argv[1]))
    {
        puts("Failed to load ROM.");
        return 1;
    }

    puts("ROM read into memory.");

    chip8_run(&chip);

    printf("\nDumping chip info:\n");
    chip8_dump(&chip);

    return 0;
}


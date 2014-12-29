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

    return 0;
}


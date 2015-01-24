#include "chip8.h"
#include <iostream>

int main(int argc, char *argv[])
{
    Chip8 chip;

    if (argc != 2)
    {
        std::cout << "Usage: chip8 rom" << std::endl;
        return 0;
    }

    if (!chip.LoadRom(argv[1]))
        return 1;

    std::cout << "ROM read into memory" << std::endl;

    chip.Run();

    return 0;
}


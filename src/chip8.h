#ifndef CHIP8_CHIP8_H_
#define CHIP8_CHIP8_H_

#include <iostream>
#include <cstdint>

struct Graphics;
struct Input;

struct Chip8
{
    Chip8();
    ~Chip8();

    bool LoadRom(const std::string& rom);
    void Run();

private:
    uint8_t* memory_;
    uint8_t* graphics_map_;
    uint8_t* V_;
    uint16_t* stack_;
    int8_t sp_;
    uint16_t I_;
    uint16_t pc_;
    uint8_t delay_timer_;
    uint8_t sound_timer_;
    bool redraw_;
    bool halt_;

    void LoadFontSet();
    void Draw(Graphics& graphics);
    void Cycle(Input& input);
};

#endif


#ifndef CHIP8_CHIP8_H_
#define CHIP8_CHIP8_H_

#include <array>
#include <cstdint>
#include <iostream>
#include <random>

struct Graphics;
struct Input;

struct Chip8
{
    Chip8();
    ~Chip8();

    bool LoadRom(const std::string& rom);
    void Run();

private:
    std::array<uint8_t, 4096> memory_;
    std::array<uint8_t, (64 * 32)> graphics_map_;
    std::array<uint8_t, 16> V_;
    std::array<uint16_t, 16> stack_;
    int8_t sp_;
    uint16_t I_;
    uint16_t pc_;
    uint8_t delay_timer_;
    uint8_t sound_timer_;
    bool redraw_;
    bool halt_;
    std::mt19937 mt_;

    void LoadFontSet();
    void Draw(Graphics& graphics);
    void Cycle(Input& input);
    uint8_t RandomByte();
};

#endif


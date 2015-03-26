cmake_minimum_required(VERSION 3.2)

project(CHIP8)

add_definitions("-std=c++11 -Weverything -Wno-c++98-compat -Wno-padded")

list(APPEND SOURCE
    src/main.cc
    src/chip8.cc
    src/graphics.cc
    src/input.cc)

add_executable(chip8 ${SOURCE})
target_link_libraries(chip8 SDL2)


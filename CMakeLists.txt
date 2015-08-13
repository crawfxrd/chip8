cmake_minimum_required(VERSION 3.2)

project(CHIP8)

add_definitions("-std=c11 -g -Weverything -Wno-padded")

list(APPEND SOURCE
    src/main.c
    src/chip8.c
    src/sdl.c)

add_executable(chip8 ${SOURCE})
target_link_libraries(chip8 SDL2)


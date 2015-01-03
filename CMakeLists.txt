cmake_minimum_required(VERSION 3.0)

project(CHIP8)

add_definitions("-Weverything -Wno-padded")

list(APPEND SOURCE
    src/main.c
    src/chip8.c
    src/opcodes.c)

add_executable(chip8 ${SOURCE})


chip8
=====

A simple CHIP-8 emulator written in C.

## Building

The CMakeLists.txt file is very simple and expects clang as the compiler.

To build, execute the following in the top level:

``` shell
mkdir build & cd build
cmake ..
make
```

This will create the `chip8` executable.

## Using

To use chip8, simply pass it a ROM file.

``` shell
./chip8 rom
```

## References

These are the sites I used to learn about CHIP-8 and assist me with my implementation.

* [Wikipedia][CHIP-8]
* [Cowgod's Chip-8 Technical Reference][Cowgod]
* [Mastering CHIP-8][mattmik]
* [How to write an emulator (CHIP-8 interpreter)][multigesture]

## License

chip8 is released under the MIT license.

[CHIP-8]: https://en.wikipedia.org/wiki/CHIP-8
[Cowgod]: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
[mattmik]: http://mattmik.com/chip8.html
[multigesture]: http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

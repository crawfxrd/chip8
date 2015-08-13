#ifndef CHIP8_OPCODES_H_
#define CHIP8_OPCODES_H_

#include "chip8.h"

void ADD(Chip8 *chip, uint16_t opcode);
void AND(Chip8 *chip, uint16_t opcode);
void CALL(Chip8 *chip, uint16_t opcode);
void CLS(Chip8 *chip);
void DRW(Chip8 *chip, uint16_t opcode);
void JMP(Chip8 *chip, uint16_t opcode);
void LD(Chip8 *chip, uint16_t opcode);
void OR(Chip8 *chip, uint16_t opcode);
void RET(Chip8 *chip);
void RND(Chip8 *chip, uint16_t opcode);
void SE(Chip8 *chip, uint16_t opcode);
void SHL(Chip8 *chip, uint16_t opcode);
void SHR(Chip8 *chip, uint16_t opcode);
void SKNP(Chip8 *chip, uint16_t opcode);
void SKP(Chip8 *chip, uint16_t opcode);
void SNE(Chip8 *chip, uint16_t opcode);
void SUB(Chip8 *chip, uint16_t opcode);
void SUBN(Chip8 *chip, uint16_t opcode);
void SYS(Chip8 *chip, uint16_t opcode);
void XOR(Chip8 *chip, uint16_t opcode);

#endif


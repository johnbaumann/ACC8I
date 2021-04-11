#ifndef CPU_H_
#define CPU_H_

#include <stdbool.h>
#include <stdint.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define RESERVED_RAM 512
#define FONT_TABLE_OFFSET 432
#define STACK_SIZE 16

// Graphics Data
const uint8_t font_table[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

struct chip8_cpu
{
    // RAM
    uint8_t memory[0xFFF];

    // Registers
    uint8_t cpureg_V[16];
    uint16_t cpureg_I;

    // Stack
    uint16_t stack[STACK_SIZE];
    uint8_t stack_index;

    // Timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    // Display
    bool screen[SCREEN_WIDTH * SCREEN_HEIGHT];

    // Program Counter
    uint16_t pc;
};

void Chip8_Initialize(struct chip8_cpu *);
void Chip8_TickCPU(struct chip8_cpu *);
void Chip8_ClearScreen(struct chip8_cpu *);
void Chip8_JumpToAddress(struct chip8_cpu *, uint16_t);
void Chip8_JumpToSubRoutine(struct chip8_cpu *, uint16_t);
void Chip8_ReturnFromSubRoutine(struct chip8_cpu *);
uint16_t Chip8_StackPop(struct chip8_cpu *);
uint16_t Chip8_StackPush(struct chip8_cpu *);

#endif // CPU_H_
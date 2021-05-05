#include "cpu.h"

#include <stdbool.h> // boolean type
#include <stdint.h> // number types
#include <stdlib.h> // rand

// References
// https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Technical-Reference
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
//

struct chip8_cpu c8_cpu;


void Chip8_Initialize(struct chip8_cpu *cpu)
{
    for (uint16_t i = 0; i < sizeof(font_table); i++)
    {
        cpu->memory[FONT_TABLE_OFFSET + i] = font_table[i];
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        cpu->cpureg_V[i] = 0;
    }

    cpu->cpureg_I = 0;

    for (uint8_t i = 0; i < STACK_SIZE; i++)
    {
        cpu->stack[i] = 0;
    }
    cpu->stack_count = 0;

    Chip8_ClearScreen(cpu);

    cpu->delay_timer = 0;
    cpu->sound_timer = 0;

    cpu->pc = 0x200;
}

void Chip8_TickCPU(struct chip8_cpu *cpu)
{
    uint16_t instruction = cpu->memory[cpu->pc] << 8 | cpu->memory[cpu->pc + 1];

    // Used for stepping through individual bits when drawing
    uint8_t bit_iterator;
    
    // Used for stepping through sprite memory when drawing
    uint16_t memory_iterator;
    
    // Used for math operations. Could re-use memory_iterator to save 2 bytes of RAM.
    uint16_t math_accumulator;
    

    switch (Nibble1(instruction))
    {
    case 0x00:
        // Execute machine language subroutine at address NNN
        // Intercept Clear the screen call
        if (instruction == 0x00E0)
        {
            Chip8_ClearScreen(cpu);
        }
        // Intercept Return from subroutine
        else if (instruction == 0x00EE)
        {
            Chip8_ReturnFromSubRoutine(cpu);
        }
        else
        {
            Chip8_JumpToSubRoutine(cpu, NNN(instruction));
        }
        break;

    case 0x01: // 1NNN
        // Jump to address NNN
        cpu->pc = NNN(instruction);
        break;

    case 0x02: // 2NNN
        // Execute subroutine starting at address NNN
        Chip8_JumpToSubRoutine(cpu, NNN(instruction));
        break;

    case 0x03: // 3XNN
        // Skip the following instruction if the value of register VX equals NN
        if (cpu->cpureg_V[Nibble2(instruction)] == NN(instruction))
        {
            cpu->pc += 2;
        }
        break;

    case 0x04: // 4XNN
        // Skip the following instruction if the value of register VX is not equal to NN
        if (cpu->cpureg_V[Nibble2(instruction)] != NN(instruction))
        {
            cpu->pc += 2;
        }
        break;

    case 0x05: // 5XY0
        // Skip the following instruction if the value of register VX is equal to the value of register VY
        if (cpu->cpureg_V[Nibble2(instruction)] == cpu->cpureg_V[Nibble3(instruction)])
        {
            cpu->pc += 2;
        }
        break;

    case 0x06: // 6XNN
        // Store number NN in register VX
        cpu->cpureg_V[Nibble2(instruction)] = NN(instruction);
        break;

    case 0x07: // 7XNN
        // Add the value NN to register VX
        cpu->cpureg_V[Nibble2(instruction)] += NN(instruction);
        break;

    case 0x08: // 8---
        switch (Nibble4(instruction))
        {
        // Store the value of register VY in register VX
        case 0x00: // 8XY0
            cpu->cpureg_V[Nibble2(instruction)] = cpu->cpureg_V[Nibble3(instruction)];
            break;

        // Set VX to VX OR VY
        case 0x01: // 8XY1
            cpu->cpureg_V[Nibble2(instruction)] |= cpu->cpureg_V[Nibble3(instruction)];
            break;

        // Set VX to VX AND VY
        case 0x02: // 8XY2
            cpu->cpureg_V[Nibble2(instruction)] &= cpu->cpureg_V[Nibble3(instruction)];
            break;

        // Set VX to VX XOR VY
        case 0x03: // 8XY3
            cpu->cpureg_V[Nibble2(instruction)] ^= cpu->cpureg_V[Nibble3(instruction)];
            break;

         
        // Add the value of register VY to register VX
        // Set VF to 01 if a carry occurs
        // Set VF to 00 if a carry does not occur
        case 0x04: // 8XY4
            math_accumulator = cpu->cpureg_V[Nibble2(instruction)] + cpu->cpureg_V[Nibble3(instruction)];
            if (math_accumulator > 0xFF)
                cpu->cpureg_V[0x0F] = 1;
            else
                cpu->cpureg_V[0x0F] = 0;

            cpu->cpureg_V[Nibble2(instruction)] = (uint8_t)(math_accumulator & 0x00FF);
            break;

        // Subtract the value of register VY from register VX
        // Set VF to 00 if a borrow occurs
        // Set VF to 01 if a borrow does not occur
        case 0x05: // 8XY5
            math_accumulator = cpu->cpureg_V[Nibble2(instruction)] - cpu->cpureg_V[Nibble3(instruction)];
            if (math_accumulator > 0xFF)
                cpu->cpureg_V[0x0F] = 1;
            else
                cpu->cpureg_V[0x0F] = 0;

            cpu->cpureg_V[Nibble2(instruction)] = (uint8_t)(math_accumulator & 0x00FF);
            break;

        // Store the value of register VY shifted right one bit in register VX¹
        // Set register VF to the least significant bit prior to the shift
        // VY is unchanged
        case 0x06: // 8XY6
            cpu->cpureg_V[0x0F] = (uint8_t)(cpu->cpureg_V[Nibble2(instruction)] & 0x01);
            cpu->cpureg_V[Nibble2(instruction)] = cpu->cpureg_V[Nibble3(instruction)] >> 1;
            break;

        // Set register VX to the value of VY minus VX
        // Set VF to 00 if a borrow occurs
        // Set VF to 01 if a borrow does not occur
        case 0x07: // 8XY7
            break;

        // Store the value of register VY shifted left one bit in register VX¹
        // Set register VF to the most significant bit prior to the shift
        // VY is unchanged
        case 0x0E: // 8XYE
            cpu->cpureg_V[0x0F] = (uint8_t)((cpu->cpureg_V[Nibble2(instruction)] & 0x80) >> 7);
            cpu->cpureg_V[Nibble2(instruction)] = cpu->cpureg_V[Nibble3(instruction)] << 1;
            break;

            //case default:
            // unimplemented
        }
        break;

    // Skip the following instruction if the value of register VX is not equal to the value of register VY
    case 0x09: // 9XY0
        if (cpu->cpureg_V[Nibble2(instruction)] != cpu->cpureg_V[Nibble3(instruction)])
        {
            cpu->pc += 2;
        }
        break;

    // Store memory address NNN in register I
    case 0x0A: // ANNN
        cpu->cpureg_I = NNN(instruction);
        break;

    // Jump to address NNN + V0
    case 0x0B: // BNNN
        cpu->pc = NNN(instruction) + cpu->cpureg_V[0];
        break;

    // Set VX to a random number with a mask of NN
    case 0x0C: // CXNN
        cpu->cpureg_V[Nibble2(instruction)] = (uint8_t)(rand() % 256) & NN(instruction);
        break;

    // Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
    // Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
    case 0x0D: // DXYN
        memory_iterator = cpu->cpureg_I;
        bit_iterator = 7;
        for (uint8_t y = cpu->cpureg_V[Nibble3(instruction)]; y - cpu->cpureg_V[Nibble3(instruction)] < Nibble4(instruction); y++)
        {
            for (uint8_t x = cpu->cpureg_V[Nibble2(instruction)]; x - cpu->cpureg_V[Nibble2(instruction)] < 8; x++)
            {
                cpu->screen[y * SCREEN_WIDTH + x] = ((cpu->memory[memory_iterator] >> bit_iterator) & 1U) > 0 ? true : false;
                //cpu->screen[y * SCREEN_WIDTH + x] = 1;
                if (bit_iterator == 0)
                {
                    bit_iterator = 7;
                    memory_iterator++;
                }
                else
                {
                    bit_iterator--;
                }
            }
        }
        break;

    case 0x0E:
        // Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed
        if ((uint8_t)(instruction & 0x00FF) == 0x9E) // EX9E
        {
            //if(KeyPressed())
                //cpu->pc += 2;
        }

        // Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed
        else if ((uint8_t)(instruction & 0x00FF) == 0xA1) // EXA1
        {
            //if(!KeyPressed())
                cpu->pc += 2;
        }
        else
        {
            // Bad opcode
        }

        break;

    case 0x0F:
        switch (NN(instruction))
        {
        // Store the current value of the delay timer in register VX
        case 0x07: // FX07
            cpu->delay_timer = cpu->cpureg_V[Nibble2(instruction)];
            break;

        // Wait for a keypress and store the result in register VX
        case 0x0A: // FX0A
            break;

        // Set the delay timer to the value of register VX
        case 0x15: // FX15
            cpu->delay_timer = cpu->cpureg_V[Nibble2(instruction)];
            break;

        // Set the sound timer to the value of register VX
        case 0x18: // FX18
            cpu->sound_timer = cpu->cpureg_V[Nibble2(instruction)];
            break;

        // Add the value stored in register VX to register I
        case 0x1E: // FX1E
            cpu->cpureg_I += cpu->cpureg_V[Nibble2(instruction)];
            break;

        // Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
        case 0x29: // FX29
            cpu->cpureg_I = FONT_TABLE_OFFSET + (Nibble2(instruction) * 5);
            break;

        // Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I + 1, and I + 2
        case 0x33: // FX33
            cpu->memory[cpu->cpureg_I] = (cpu->cpureg_V[Nibble2(instruction)] % 1000) / 100;
            cpu->memory[cpu->cpureg_I + 1] = (cpu->cpureg_V[Nibble2(instruction)] % 100) / 10;
            cpu->memory[cpu->cpureg_I + 2] = (cpu->cpureg_V[Nibble2(instruction)] % 10);
            break;

        // Store the values of registers V0 to VX inclusive in memory starting at address I
        // I is set to I + X + 1 after operation²
        case 0x55: // FX55
            for (uint8_t i = 0; i < Nibble2(instruction); i++)
            {
                cpu->memory[cpu->cpureg_I] = cpu->cpureg_V[i];
                cpu->cpureg_I++;
            }
            break;

        // Fill registers V0 to VX inclusive with the values stored in memory starting at address I
        // I is set to I + X + 1 after operation²
        case 0x65: // FX65
            for (uint8_t i = 0; i < Nibble2(instruction); i++)
            {
                cpu->cpureg_V[i] = cpu->memory[cpu->cpureg_I];
                cpu->cpureg_I++;
            }
            break;
        }
        break;

    default:
        // Default error handling
        cpu->pc = cpu->pc; // Do something cause unimplemented
        printf("unknown instruction %X\n", instruction);
    }

    // Check instruction types, skip PC increment if jump type
    switch (Nibble1(instruction))
    {
    case 0x00:
        // Bad code flow
        // Increment pc on clear screen and return from sub-routine
        if (Nibble3(instruction) == 0x0E)
            cpu->pc += 2;
    case 0x01:
    case 0x02:
    case 0x0B:
        break;

    default:
        cpu->pc += 2;
    }
}

void Chip8_ClearScreen(struct chip8_cpu *cpu)
{
    for (uint16_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        cpu->screen[i] = 0;
    }
}

static void Chip8_JumpToSubRoutine(struct chip8_cpu *cpu, uint16_t address)
{
    // Push pc to stack
    Chip8_StackPush(cpu, cpu->pc);
    cpu->pc = address;
}

static void Chip8_ReturnFromSubRoutine(struct chip8_cpu *cpu)
{
    // Pull pc from stack
    cpu->pc = Chip8_StackPop(cpu);
}

static uint16_t Chip8_StackPop(struct chip8_cpu *cpu)
{
    uint16_t stack_value;

    if (cpu->stack_count == 0)
    {
        // Stack under-flow condition, return
        stack_value = 0x200;
    }
    else
    {
        stack_value = cpu->stack[cpu->stack_count - 1];
        cpu->stack_count--;
    }

    return stack_value;
}

static void Chip8_StackPush(struct chip8_cpu *cpu, uint16_t value)
{
    cpu->stack_count++;
    if (cpu->stack_count >= STACK_SIZE)
    {
        // Stack over-flow condition
        //printf("Error: Stack Overflow\n");
        cpu->stack_count = 1;
    }
    cpu->stack[cpu->stack_count-1] = value;
}

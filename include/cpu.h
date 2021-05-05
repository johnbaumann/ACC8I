#ifndef CPU_H_
#define CPU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define RESERVED_RAM 512
#define FONT_TABLE_OFFSET 432
#define STACK_SIZE 12

	// Returns the first nibble from the left of a 16-bit unsigned integer.
	inline uint8_t Nibble1(uint16_t instruction)
	{
		return (uint8_t)((instruction & 0xF000) >> 12);
	}

	// Returns the second nibble from the left of a 16-bit unsigned integer.
	inline uint8_t Nibble2(uint16_t instruction)
	{
		return (uint8_t)((instruction & 0x0F00) >> 8);
	}

	// Returns the third nibble from the left of a 16-bit unsigned integer.
	inline uint8_t Nibble3(uint16_t instruction)
	{
		return (uint8_t)((instruction & 0x00F0) >> 4);
	}

	// Returns the fourth nibble from the left of a 16-bit unsigned integer.
	inline uint8_t Nibble4(uint16_t instruction)
	{
		return (uint8_t)(instruction & 0x000F);
	}

	inline uint8_t N(uint16_t instruction)
	{
		return (uint8_t)(instruction & 0x000F);
	}

	inline uint8_t NN(uint16_t instruction)
	{
		return (uint8_t)(instruction & 0x00FF);
	}

	inline uint16_t NNN(uint16_t instruction)
	{
		return (uint16_t)(instruction & 0x0FFF);
	}

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
		uint8_t memory[4096];

		// Data registers[16]
		uint8_t cpureg_V[16];
		// Address register(12-bits usable)
		uint16_t cpureg_I;

		// Stack
		uint16_t stack[STACK_SIZE];
		int8_t stack_count;

		// Delay timer
		uint8_t delay_timer;
		// Sound Timer
		uint8_t sound_timer;

		// Display
		bool screen[SCREEN_WIDTH * SCREEN_HEIGHT];

		// Program Counter
		uint16_t pc;
	};

	void Chip8_Initialize(struct chip8_cpu*);
	void Chip8_TickCPU(struct chip8_cpu*);
	void Chip8_ClearScreen(struct chip8_cpu*);
	static void Chip8_JumpToSubRoutine(struct chip8_cpu*, uint16_t);
	static void Chip8_ReturnFromSubRoutine(struct chip8_cpu*);
	static uint16_t Chip8_StackPop(struct chip8_cpu*);
	static void Chip8_StackPush(struct chip8_cpu*, uint16_t);

	extern struct chip8_cpu c8_cpu;

#ifdef __cplusplus
}
#endif

#endif // CPU_H_


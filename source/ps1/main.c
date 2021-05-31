#include <sys/types.h>
#include <libetc.h>

#include <stdio.h>
//#include <libapi.h> // Root counters
#include <stdint.h>

#include "ps1/filesystem.h"
#include "ps1/graphics.h"
#include "ps1/pads.h"
#include "shared/chip8_cpu.h"

extern uint8_t chip8_rom[];

bool program_paused;
uint8_t cpu_rate;

void LoadROM();

void LoadROM()
{
	for (int i = 0; i < 3203; i++)
	{
		c8_cpu.memory[0x200 + i] = chip8_rom[i];
	}
}

int main()
{
	int activeBuffer = 0;
	int cpu_ticks_per_frame;

	cpu_rate = 1U;
	program_paused = false;

	InitPads();
	InitGraphics();

	Chip8_Initialize(&c8_cpu);
	InitFilesystem();
	LoadFile(".\\thirdparty\\chip8-roms\\games\\Tetris [Fran Dachille, 1991].ch8", &c8_cpu);
	//LoadROM();

	if (GetVideoMode() == MODE_NTSC)
	{
		// 500Hz / 60Hz = 8.33 ticks
		cpu_ticks_per_frame = 8;
	}
	else
	{
		// 500Hz / 50Hz = 10 ticks
		cpu_ticks_per_frame = 10;
	}

	while (1)
	{
		FntPrint("%d FPS, CPU RATE %dx\n", frame_rate, cpu_rate);
		FntPrint("Tetris [Fran Dachille, 1991]\n");

		UpdatePads();
		HandleSystemPadEvents();

		if (!program_paused)
		{
			HandleCHIP8KeyboardEvents();
			for (int j = 0; j < cpu_rate; j++)
			{
				for (int i = 0; i < cpu_ticks_per_frame; i++)
				{
					Chip8_TickCPU(&c8_cpu);
				}
				// 60Hz is target
				// 50Hz from PAL should be close enough to not be a concern.
				Chip8_UpdateTimers(&c8_cpu);
			}

			// Update CHIP8 texture render
			UpdateScreenTexture();
		}
		DisplayAll();
	}
	return 0;
}

// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "cpu.h"
#include "gui.h"
#include <iostream>



int main(int, char**)
{
	bool program_running = true;
	long file_size;

	if (GUI_Init() > 0)
		return -1;

	Chip8_Initialize(&c8_cpu);

	FILE* inFile = fopen("E:\\CHIP8\\test_opcode.ch8", "rb");
	if (inFile == NULL)
	{
		printf("Error opening file!\n");
		return -1;
	}
	else
	{
		fseek(inFile, 0, SEEK_END);
		file_size = ftell(inFile);
		if (file_size > 3583)
		{
			printf("File size too large\n");
			return -1;
		}
		else if (file_size <= 0)
		{
			printf("File is empty\n");
			return -1;
		}
		rewind(inFile);

		//while (ftell(inFile) < file_size)
		for (int i = 0; i < file_size; i++)
		{
			int data_in = fgetc(inFile);
			if (data_in == EOF)
				break;
			c8_cpu.memory[i + 0x200] = (uint8_t)data_in;
		}

		// Main loop
		while (program_running)
		{
			if (!GUI_Render())
				break;
		}

		GUI_Closing();

		return 0;
	}
}
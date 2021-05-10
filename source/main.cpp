// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "cpu.h"
#include "gui.h"


int main(int, char**)
{
	bool program_running = true;

	if (GUI_Init() > 0)
		return -1;

	Chip8_Initialize(&c8_cpu);
	//OpenROM("thirdparty\\chip8-roms\\programs\\BMP Viewer - Hello (C8 example) [Hap, 2005].ch8");


	// Main loop
	while (program_running)
	{
		if (!GUI_Render())
			break;
	}

	GUI_Closing();

	return 0;
}
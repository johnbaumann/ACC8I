#include "cpu.h"

#include <SDL2/SDL.h>
#include <iostream>

chip8_cpu c8_cpu;
bool isRunning = true;

void Process_Input()
{
	SDL_Event sdl_event;

	while (SDL_PollEvent(&sdl_event))
	{
		switch (sdl_event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;

		case SDL_KEYDOWN:
			if (sdl_event.key.keysym.sym == SDLK_ESCAPE)
			{
				isRunning = false;
			}
		}
	}
}

int main(int argv, char **args)
{
	uint16_t fps_target = 60;
	uint32_t millis_per_frame = (fps_target > 0) ? (1000 / fps_target) : 0;

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *sdl_window = SDL_CreateWindow("ACC8I - A Crappy CHIP-8 Interpreter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, 0);
	SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);

	Chip8_Initialize(&c8_cpu);

	uint32_t render_start_time;
	uint32_t render_end_time;

	while (isRunning)
	{
		render_start_time = SDL_GetTicks();

		Process_Input();

		SDL_RenderClear(sdl_renderer);
		SDL_SetRenderDrawColor(sdl_renderer, 240, 240, 240, 255);
		SDL_RenderPresent(sdl_renderer);

		render_end_time = SDL_GetTicks();

		if (millis_per_frame > 0)
		{
			if (render_end_time - render_start_time < millis_per_frame)
			{
				SDL_Delay((render_start_time + millis_per_frame) - render_end_time);
			}
		}
	}

	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	return 0;
}
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "chip8.h"
#include "constants.h"
#include "utility.h"
#include <iostream>
#include <chrono>

chip8 myChip8;

SDL_Window* window{};
SDL_Renderer* renderer{};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	SDL_SetAppMetadata("chip8", "1.0", "com.example.chip8");
	
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!SDL_CreateWindowAndRenderer("chip8", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
		SDL_Log("Couldn't initialize window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	myChip8.initialize();
	if (argc < 2) {
		std::cout << "Usage: [program_name] [game_path]";
		return SDL_APP_FAILURE;
	}
	myChip8.loadGame(argv[1]);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;
	return SDL_APP_CONTINUE;
}

constexpr int fps{ 60 };

// number of opcodes to execute a second
constexpr int numopcodes{ 500 };

// number of opcodes to execute a frame 
constexpr int numframe{ numopcodes / fps };

constexpr float interval{ 1000 / fps };

Uint64 time2{ SDL_GetTicks() };

SDL_AppResult SDL_AppIterate(void* appstate) {
	Uint64 current = SDL_GetTicks();
	if ((time2 + interval) < current)
	{
		myChip8.decreaseTimers();
		for (int i = 0; i < numframe; i++) {
			myChip8.emulateCycle();
			if (myChip8.drawFlag) {
				drawGrapics(renderer, myChip8.gfx);
				myChip8.drawFlag = false;
			}
			myChip8.setKeys();
		}
		time2 = current;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {

}


#ifndef UTILITY_H
#define UTILITY_H

#include <SDL3/SDL.h>
#include "chip8.h"
#include "constants.h"
#include <vector>

/**
	* Draws pixels from chip8's gfx member array using SDL library.
	* \param renderer rendering context (SDL library)
	* \param gfx chip8's gfx (graphics) member array
*/
void drawGrapics(SDL_Renderer*& renderer, const unsigned char* gfx) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	std::vector<SDL_FPoint> points;

	for (int y{ 0 }; y < 32; ++y)
		for (int x{ 0 }; x < 64; ++x)
			if (gfx[x + y * 64])
			{
				for (int yy{ y * HEIGHT_MULTIPLIER }; yy < (y + 1) * HEIGHT_MULTIPLIER; ++yy)
					for (int xx{ x * WIDTH_MULTIPLIER }; xx < (x + 1) * WIDTH_MULTIPLIER; ++xx) {
						points.push_back({ static_cast<float>(xx), static_cast<float>(yy) });
					}
			}
	SDL_RenderPoints(renderer, points.data(), static_cast<int>(points.size()));

	SDL_RenderPresent(renderer);
}

#endif // !UTILITY_H

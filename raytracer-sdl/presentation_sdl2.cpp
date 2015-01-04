#include "presentation_sdl2.h"
#include "config.h"
#include <SDL2/SDL.h>


#define REQUIRE(cond) do { if(!(cond)) { printf("Cond failed %s: %s\n", __FILE__, __LINE__); exit(0); }  }while(0)

namespace rt {
	namespace sdl {


		static void putpixel(SDL_Surface *surface, int x, int y, core::Color pixel)
		{
			int bpp = surface->format->BytesPerPixel;
			/* Here p is the address to the pixel we want to set */
			Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

			switch (bpp) {
			case 4:
				if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
					p[0] = pixel.r;
					p[1] = pixel.g;
					p[2] = pixel.b;
				}
				else {
					p[0] = pixel.b;
					p[1] = pixel.g;
					p[2] = pixel.r;
				}
				break;
			}
		}


		void present_rendering(std::vector<rt::core::Surface2d*>& surfaces) {
			if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				printf("Failed to init SDL");
				exit(0);
			}
			SDL_Window* window = SDL_CreateWindow("Raytracer", SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED, WND_SIZE_X, WND_SIZE_Y, 0);
			REQUIRE(window);

			//make sure we do not have to do some kind of resizing
			//later may be supported but for *now* - no
			int w = 0, h = 0;
			SDL_GetWindowSize(window, &w, &h);
			REQUIRE(w == WND_SIZE_X);
			REQUIRE(h == WND_SIZE_Y);


			SDL_Surface* surface = SDL_GetWindowSurface(window);
			REQUIRE(surface);

			int current_image = 0;
			bool running = true;
			while (running) {

				SDL_Event e;
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT) {
						running = false;
					}
					else if (e.type == SDL_KEYDOWN) {
						if (e.key.keysym.sym == SDLK_n) {
							current_image++;
							if (current_image >= surfaces.size()) {
								current_image = 0;
							}
						}
					}
				}
				rt::core::Surface2d* to_copy = surfaces[current_image];

				SDL_LockSurface(surface);
				for (int y = 0; y < h; ++y) {
					for (int x = 0; x < w; ++x) {
						putpixel(surface, x, y, to_copy->pixel(x, y));
					}
				}
				SDL_UnlockSurface(surface);

				SDL_UpdateWindowSurface(window);
				SDL_Delay(120);
			}

			SDL_DestroyWindow(window);
			SDL_Quit();
		}
	}
}
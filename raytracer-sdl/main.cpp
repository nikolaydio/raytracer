#include <scene.h>
#include <camera.h>

#include <SDL.h>


#define WND_SIZE_X 640
#define WND_SIZE_Y 480

#define REQUIRE(cond) do { if(!(cond)) { printf("Cond failed %s: %s\n", __FILE__, __LINE__); exit(0); }  }while(0)

int main(int argc, char* argv[]) {
	rt::core::Film film(WND_SIZE_X, WND_SIZE_Y);
	rt::core::Camera cam;
	rt::core::Scene scene;

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

	uint32_t* pixels = (uint32_t*)surface->pixels;
	bool running = true;
	while (running) {

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT) {
				running = false;
			}
		}

		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				pixels[y * w + x] = *(uint32_t*)&film.pixel(x, y);
			}
		}

		SDL_UpdateWindowSurface(window);
		SDL_Delay(120);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
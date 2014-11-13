#include "renderer.h"

#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>


#define WND_SIZE_X 640
#define WND_SIZE_Y 480

#define REQUIRE(cond) do { if(!(cond)) { printf("Cond failed %s: %s\n", __FILE__, __LINE__); exit(0); }  }while(0)

bool IntersectSphere(glm::vec3 sphere_pos, float radius, glm::vec3 rpos, glm::vec3 rdir, float* dist_out) {
	glm::vec3 L = sphere_pos - rpos;
	float tca = glm::dot(L, rdir);
	if (tca < 0) return false;
	float d2 = glm::dot(L, L) - tca * tca;
	if (d2 > radius) return false;
	float thc = glm::sqrt(radius - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;


	if (t0 > t1) {
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	if (t1 < 0)
		return false;

	if (t0 < 0){
		*dist_out = t1;
		return true;
	}
	else{
		*dist_out = t0;
		return true;
	}
}

class Sphere : public rt::core::Shape {
public:
	virtual bool intersect(glm::mat4 transform, rt::core::Ray ray, rt::core::Intersection* result) const {
		glm::vec4 pos = transform[3];
		float radius = transform[0][0];
		if (IntersectSphere(glm::vec3(pos), radius, ray.origin, ray.orientation, &result->d)) {
			result->normal = (ray.origin + ray.orientation * result->d) - glm::vec3(pos);
			result->normal = glm::normalize(result->normal);
			return true;
		}
		return false;
	}
};

int main(int argc, char* argv[]) {

	rt::core::Surface2d film_surface(WND_SIZE_X, WND_SIZE_Y);

	rt::core::Film film(&film_surface);

	rt::core::Camera cam(glm::lookAt(glm::vec3(0, 0, -3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), 60, 4.f / 3.f);
	rt::core::Scene scene;
	
	rt::core::Material material;
	material.color = glm::vec3(0, 1, 0);
	Sphere sphere;
	rt::core::Primitive primitive(glm::translate(glm::mat4(), glm::vec3(0,0,3)), &material, &sphere);
	scene.add_primitive(&primitive);

	rt::core::Sampler sampler;
	rt::core::Integrator integrator;

	rt::core::Renderer renderer(sampler, cam, scene, integrator);
	renderer.film() = &film;

	renderer.run_singlethreaded();

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
				pixels[y * w + x] = *(uint32_t*)&film_surface.pixel(x, y);
			}
		}

		SDL_UpdateWindowSurface(window);
		SDL_Delay(120);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
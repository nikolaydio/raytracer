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
	glm::vec3 _pos;
	float _radius;
public:
	Sphere(glm::vec3 pos, float radius)
		: _pos(pos), _radius(radius) {}
	virtual bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
		if (IntersectSphere(_pos, _radius, ray.origin, ray.orientation, &result->d)) {
			result->normal = (ray.origin + ray.orientation * result->d) - _pos;
			result->normal = glm::normalize(result->normal);
			return true;
		}
		return false;
	}
	rt::core::AABB get_bounding_box() const {
		return rt::core::AABB(_pos - _radius, _pos + _radius);
	}
};


void build_scene(rt::core::Scene* scene) {
	rt::core::Material mat;
	mat.emitted = glm::vec3(0.2, 0.86, 0.45);
	mat.reflected = glm::vec3(0.3, 0.9, 0.7);
	rt::core::MaterialId left_sph_mat = scene->new_material(mat);

	mat.emitted = glm::vec3(0.9, 0.0, 0.0);
	mat.reflected = glm::vec3(0.9, 0.04, 0.7);
	rt::core::MaterialId right_sph_mat = scene->new_material(mat);

	Sphere* sphere = new Sphere(glm::vec3(10, 10, 10), 1);
	rt::core::GeoPrimitive* prim = new rt::core::GeoPrimitive(sphere, left_sph_mat);
	scene->add_primitive(prim);

	sphere = new Sphere(glm::vec3(12, 9.6, 10), 1.25);
	prim = new rt::core::GeoPrimitive(sphere, right_sph_mat);
	scene->add_primitive(prim);
}

int main(int argc, char* argv[]) {

	rt::core::Surface2d film_surface(WND_SIZE_X, WND_SIZE_Y);

	rt::core::Film film(&film_surface);

	rt::core::Camera cam(glm::vec3(10.5, 10.5, 3), glm::vec3(11, 10, 10), 60, (float)WND_SIZE_X / (float)WND_SIZE_Y);

	rt::core::Scene scene;
	build_scene(&scene);

	scene.accelerate_and_rebuild(new rt::core::DefaultAccelerator);

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
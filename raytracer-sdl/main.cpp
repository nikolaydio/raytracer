#include "renderer.h"
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>

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
		if (IntersectSphere(_pos, _radius, ray.origin, ray.direction, &result->d)) {
			result->position = (ray.origin + ray.direction * result->d);
			result->normal = glm::normalize(result->position - _pos);
			return true;
		}
		return false;
	}
	rt::core::AABB get_bounding_box() const {
		return rt::core::AABB(_pos - _radius, _pos + _radius);
	}
};

class Triangle : public rt::core::Shape {
	glm::vec3 p0, p1, p2;
public:
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) : p0(a), p1(b), p2(c) {}

	bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
		glm::vec3 e1 = p1 - p0;
		glm::vec3 e2 = p2 - p0;
		glm::vec3 s1 = glm::cross(ray.direction, e2);
		float divisor = glm::dot(s1, e1);

		//this line is looking suspicious
		if (divisor == 0.) {
			return false;
		}
		float invDivisor = 1.f / divisor;
		//first barycentric coordinate
		glm::vec3 d = ray.origin - p0;
		float b1 = glm::dot(d, s1) * invDivisor;
		if (b1 < 0. || b1 > 1.) {
			return false;
		}
		//second
		glm::vec3 s2 = glm::cross(d, e1);
		float b2 = glm::dot(ray.direction, s2) * invDivisor;
		if (b2 <0. || b1 + b2 > 1.) {
			return false;
		}
		float t = glm::dot(e2, s2) * invDivisor;
		//this is a threashhold for not colliding with itself
		if (t < 0.0001) {
			return false;
		}
		result->d = t;
		result->position = ray.origin + ray.direction * t;
		result->normal = glm::normalize(glm::cross(e1, e2));
		return true;
	}
	rt::core::AABB get_bounding_box() const {
		return rt::core::AABB(p0, p1, p2);
	}
};

void build_scene(rt::core::Scene* scene) {
	rt::core::Material mat;
	mat.emitted = glm::vec3(0.2, 0.86, 0.45);
	mat.reflected = glm::vec3(0.2, 0.86, 0.45);
	rt::core::MaterialId left_sph_mat = scene->new_material(mat);

	mat.emitted = glm::vec3(0.69, 0.12, 0.164);
	mat.reflected = glm::vec3(0.9, 0.04, 0.7);
	rt::core::MaterialId right_sph_mat = scene->new_material(mat);

	mat.emitted = glm::vec3(1, 1, 1);
	mat.reflected = glm::vec3(0.4, 0.4, 0.4);
	rt::core::MaterialId right_tri_mat = scene->new_material(mat);

	mat.emitted = glm::vec3(0.14, 0.15, 0.37);
	mat.reflected = glm::vec3(0.6, 0.6, 0.6);
	rt::core::MaterialId blue = scene->new_material(mat);


	mat.emitted = glm::vec3(0.98, 0.98, 0.98);
	mat.reflected = glm::vec3(0.1, 0.1, 0.1);
	rt::core::MaterialId white = scene->new_material(mat);

	rt::core::Shape* shape = new Sphere(glm::vec3(0, 0, 0), 1);
	rt::core::GeoPrimitive* prim = new rt::core::GeoPrimitive(shape, left_sph_mat);
	scene->add_primitive(prim);

	shape = new Sphere(glm::vec3(2, -0.6, 0), 1.25);
	prim = new rt::core::GeoPrimitive(shape, right_sph_mat);
	scene->add_primitive(prim);

	shape = new Sphere(glm::vec3(1.5, -2, 0), 0.65);
	prim = new rt::core::GeoPrimitive(shape, white);
	scene->add_primitive(prim);


	shape = new Triangle(glm::vec3(15,-5,10), glm::vec3(15, 5, 9), glm::vec3(15, -5, 5));
	prim = new rt::core::GeoPrimitive(shape, right_tri_mat);
	scene->add_primitive(prim);

	float pos = 100;
	shape = new Triangle(glm::vec3(-300, -300, pos), glm::vec3(0, 200, pos), glm::vec3(300, -300, pos));
	prim = new rt::core::GeoPrimitive(shape, blue);
	scene->add_primitive(prim);
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

void test() {
	Triangle a(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
	rt::core::Intersection isect;
	rt::core::Ray ray;
	ray.origin = glm::vec3(0.1, 0.1, -1);
	ray.direction = glm::vec3(0, 0, 1);
	assert(a.intersect(ray, &isect));
	assert(isect.normal == glm::vec3(0,0,-1));
	assert(isect.position == glm::vec3(0.1, 0.1, 0));

	ray.origin = glm::vec3(1, 1, -1);
	assert(!a.intersect(ray, &isect));

	ray.origin = glm::vec3(0.1, 0.1, 1);
	ray.direction = glm::vec3(0, 0, -1);
	assert(a.intersect(ray, &isect));
	assert(isect.normal == glm::vec3(0, 0, -1));
	assert(isect.position == glm::vec3(0.1, 0.1, 0));
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	test();
#endif

	rt::core::Surface2d film_surface(WND_SIZE_X, WND_SIZE_Y);

	rt::core::Film film(&film_surface);

	rt::core::Camera cam(glm::vec3(1, -0.1, -5), glm::vec3(1.5, -0.3, 0), 60, (float)WND_SIZE_X / (float)WND_SIZE_Y);

	rt::core::Scene scene;
	build_scene(&scene);

	scene.accelerate_and_rebuild(new rt::core::DefaultAccelerator);

	rt::core::Sampler sampler(1);
	rt::core::Integrator integrator;

	rt::core::Renderer renderer(sampler, cam, scene, integrator);
	renderer.film() = &film;

	auto begin = std::chrono::high_resolution_clock::now();
	renderer.run_multithreaded();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
	double seconds = (double)duration / 1000.0 / 1000.0 / 1000.0;
	std::cout << "Render time: " << seconds << std::endl;

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


	bool running = true;
	while (running) {

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT) {
				running = false;
			}
		}

		SDL_LockSurface(surface);
		uint32_t* pixels = (uint32_t*)surface->pixels;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				putpixel(surface, x, y, *(Uint32*)&film_surface.pixel(x, y));
			}
		}
		SDL_UnlockSurface(surface);

		SDL_UpdateWindowSurface(window);
		SDL_Delay(120);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
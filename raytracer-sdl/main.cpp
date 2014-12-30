#if defined(_DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif


#include "renderer.h"
#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>

#include "shape.h"
#include "scene_loader.h"
#include "path.h"
#include <thread>

#include "film.h"
#include "presentation_sdl2.h"

#include <memory>



void* rendering_thread(std::vector<rt::core::Renderer>* renderers) {
	for (int i = 0; i < renderers->size(); ++i) {
		auto begin = std::chrono::high_resolution_clock::now();
		(*renderers)[i].run_multithreaded();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
		double seconds = (double)duration / 1000.0 / 1000.0 / 1000.0;
		std::cout << "Image " << i << " Render time: " << seconds << std::endl;
	}
	return 0;
}

#define RENDER_CONTEXT_MEMORY_ARENA_SIZE 64*1024


int raytracer_entry_point(int argc, char* argv[]) {
	rt::core::Scene scene;
	const char* scene_fn = "/default.scene";
	if (argc > 1) {
		scene_fn = argv[1];
	}
	
	rt::sdl::FileLoader loader;
	loader.add_directory("./scenes");
	auto&& file = rt::sdl::load_config_file(scene_fn, loader);
	if (!file) {
		return 0;
	}

	rt::sdl::ResourceManager manager(loader);
	if (!rt::sdl::load_scene_and_accelerate(*file.get(), scene, manager)) {
		std::cout << "Failed to load scene " << scene_fn << std::endl;
		return 0;
	}
	
	std::vector<rt::core::Renderer> renderers;
	std::vector<rt::core::Film*> films;
	rt::core::MemoryArena arena(RENDER_CONTEXT_MEMORY_ARENA_SIZE);
	if (!rt::sdl::load_images(*file.get(), renderers, scene, films, arena)) {
		return 0;
	}
	file.reset();


	std::thread render_thread(rendering_thread, &renderers);

	std::vector<rt::core::Surface2d*> surfaces;
	for (auto film : films) {
		surfaces.push_back(film->get_surface());
	}
	rt::sdl::present_rendering(surfaces);
	
	for (auto& renderer : renderers) {
		renderer.do_not_render();
	}
	render_thread.join();
	for (auto film : films) {
		delete film;
	}
	manager.cleanup();
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	std::cout << "Warning: This is a debug build\n" << std::endl;
#endif
	int result = raytracer_entry_point(argc, argv);
#if defined(_DEBUG) && defined(WIN32)
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
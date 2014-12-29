#pragma once
#include "scene.h"
#include "renderer.h"
#include "resource_manager.h"

namespace rt {
	namespace sdl {
		typedef void* config_file;
		config_file load_config_file(const char* fn, FileLoader& loader);
		void free_config_file(config_file f);

		bool load_scene_and_accelerate(config_file f, rt::core::Scene& scene, ResourceManager& manager);
		bool load_images(config_file f, std::vector<rt::core::Renderer>& renderers, rt::core::Scene& scene, std::vector<rt::core::Surface2d*>& surfaces, core::MemoryArena& arena);
	}
}
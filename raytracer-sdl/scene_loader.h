#pragma once
#include "scene.h"
#include "renderer.h"
#include "resource_manager.h"
#include <memory>

namespace rt {
	namespace sdl {
		struct ConfigFile;
		class ConfigFileDeleter {
		public:
			void operator()(ConfigFile* p);
		};

		std::unique_ptr<ConfigFile, ConfigFileDeleter> load_config_file(const char* fn, FileLoader& loader);
		void free_config_file(ConfigFile* f);

		bool load_scene_and_accelerate(const ConfigFile& f, rt::core::Scene& scene, ResourceManager& manager);
		bool load_images(const ConfigFile& f, std::vector<rt::core::Renderer>& renderers, rt::core::Scene& scene, std::vector<rt::core::Film*>& films, core::MemoryArena& arena);
	}
}
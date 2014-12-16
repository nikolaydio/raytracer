#pragma once
#include "scene.h"
#include "resource_manager.h"

namespace rt {
	namespace sdl {
		class SceneLoader {
		public:
			static bool load_from(const char* filename, rt::core::ResourceManager& manager, core::Scene& scene);
		};
	}
}
#pragma once
#include "renderer.h"

namespace rt {
	namespace core {
		class Path : public Integrator {
		public:
			glm::vec3 calculate_color(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, int depth, MemoryArena& arena) const;
			virtual Spectrum calculate_radiance(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, MemoryArena& arena) const;
		};
	}
}
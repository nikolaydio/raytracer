#pragma once
#include "renderer.h"
#include "rng.h"

namespace rt {
	namespace core {
		class Path : public Integrator {
		public:
			glm::vec3 calculate_color(const Scene& scene, Ray ray, Intersection isect, int depth, RNG& rng, MemoryArena& arena) const;
			virtual Spectrum calculate_radiance(const Scene& scene, Ray ray, Intersection isect, RNG& rng, MemoryArena& arena) const;
		};
	}
}
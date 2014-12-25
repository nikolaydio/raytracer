#include "path.h"
#include "material.h"

namespace rt {
	namespace core {

		Spectrum Path::calculate_color(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, int depth, MemoryArena& arena) const {
			if (depth > 2) {
				return glm::vec3(0, 0, 0);
			}
			Material mat = man.material(isect.material);
			Spectrum emitted = mat.emitted;
			if (glm::length(emitted) > 1.7) {
				return emitted;
			}

			Spectrum incident = glm::vec3(0, 0, 0);
			Intersection nested;
			Ray nray;
			Spectrum reflected(0, 0, 0);

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(0, 1);

			glm::vec3 outgoing = -ray.direction;
			for (int i = 0; i < 6; ++i) {
				nray.origin = isect.position;
				glm::vec3 incident;
				float pdf;
				BSDF* brdf = mat.get_brdf(isect, arena);
				Spectrum brdf_val = brdf->evaluate_sample_f(outgoing, &nray.direction, dis(gen), dis(gen), &pdf);
				if (scene.intersect(nray, &nested)) {
					incident = calculate_color(man, scene, nray, nested, depth + 1, arena);
					float coef = glm::dot(isect.normal, nray.direction);

					reflected += (brdf_val * incident * coef) / pdf;
				}
			}
			reflected /= 6.0f;

			return glm::clamp(emitted + reflected, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
		}
		Spectrum Path::calculate_radiance(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, MemoryArena& arena) const {
			return calculate_color(man, scene, ray, isect, 0, arena);
		}
	}
}
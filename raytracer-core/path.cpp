#include "path.h"
#include "material.h"

namespace rt {
	namespace core {
		//It will be runned for each sample so generally this is good enough even if its set to 1
#define SAMPLE_LIGHT_COUNT 1
		Spectrum Path::calculate_color(const Scene& scene, Ray ray, Intersection isect, int depth, RNG& rng, MemoryArena& arena) const {
			

			Spectrum path_throughput(1, 1, 1), L(0,0,0);
			const int max_depth = 3;
			for (int bounces = 0; ; ++bounces) {

				const Material& mat = scene.material_by_id(isect.material);
				L += mat.emitted * path_throughput;
				BSDF* bsdf = mat.get_brdf(isect, arena);

				glm::vec3 outgoing = -ray.direction;

				//Sample direct lights
				Spectrum sampled_light_contribution;
				for (int i = 0; i < SAMPLE_LIGHT_COUNT; ++i) {
					sampled_light_contribution += uniform_sample_one_light(scene, isect, bsdf, outgoing, rng, arena);
				}
				L += (sampled_light_contribution / (float)SAMPLE_LIGHT_COUNT) * path_throughput ;

				glm::vec3 incident;
				float pdf = 0;
				Spectrum f = bsdf->evaluate_sample_f(outgoing, &incident, rng.gen(), rng.gen(), &pdf);
				if (glm::length(f) < 0.000001 || pdf == 0.) {
					break;
				}



				path_throughput *= f * glm::abs(glm::dot(incident, isect.normal)) / pdf;

				//check lights
				if (bounces == max_depth) {
					break;
				}

				ray.origin = isect.position;
				ray.direction = incident;
				if (!scene.intersect(ray, &isect)) {
					break;
				}
			}
			return L;
		}
		Spectrum Path::calculate_radiance(const Scene& scene, Ray ray, Intersection isect, RNG& rng, MemoryArena& arena) const {
			return calculate_color(scene, ray, isect, 0, rng, arena);
		}
	}
}
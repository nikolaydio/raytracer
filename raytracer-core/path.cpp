#include "path.h"
#include "material.h"

namespace rt {
	namespace core {
		//It will be runned for each sample so generally this is good enough even if its set to 1
#define SAMPLE_LIGHT_COUNT 1
		Spectrum Path::calculate_color(const Scene& scene, Ray ray, Intersection isect, int depth, RNG& rng, MemoryArena& arena) const {
			//std::random_device rd;
			//std::mt19937 gen(rd());
			//std::minstd_rand gen(0);
			//std::uniform_real_distribution<float> dis(0, 1); 
			

			Spectrum path_throughput(1, 1, 1), L(0,0,0);
			const int max_depth = 3;
			for (int bounces = 0; ; ++bounces) {

				const Material& mat = scene.material_by_id(isect.material);
				L += mat.emitted * path_throughput;
				BSDF* bsdf = mat.get_brdf(isect, arena);

				glm::vec3 outgoing = -ray.direction;

				//Sample lights
				Spectrum sampled_light_contribution;
				for (int i = 0; i < SAMPLE_LIGHT_COUNT; ++i) {
					//choose random emitting object in the scene
					Node& node = scene.sample_light(rng.gen());
					//choose random position on the surface of this object
					glm::vec3 light_pos = node.sample_position(rng.gen(), rng.gen(), rng.gen());
					glm::vec3 light_incident = light_pos - isect.position;
					float light_distance = glm::length(light_incident);
					light_incident = glm::normalize(light_incident);

					Intersection temp;
					Ray tray; tray.origin = isect.position; tray.direction = light_incident;
					if (!scene.intersect(tray, &temp)) {
						continue;
					}
					if (temp.d + 0.0000001 < light_distance) {
						continue;
					}
					sampled_light_contribution += path_throughput * bsdf->evaluate_f(outgoing, light_incident) * glm::abs(glm::dot(light_incident, isect.normal));

				}
				L += sampled_light_contribution / (float)SAMPLE_LIGHT_COUNT;

				glm::vec3 incident;
				float pdf;
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
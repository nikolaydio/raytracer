#include "path.h"
#include "material.h"

namespace rt {
	namespace core {

		Spectrum Path::calculate_color(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, int depth, MemoryArena& arena) const {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(0, 1); 
			

			Spectrum path_throughput(1, 1, 1), L(0,0,0);
			const int max_depth = 2;
			for (int bounces = 0; ; ++bounces) {

				Material& mat = man.material(isect.material);
				L += mat.emitted * path_throughput;
				BSDF* bsdf = mat.get_brdf(isect, arena);

				glm::vec3 outgoing = -ray.direction;
	

				glm::vec3 incident;
				float pdf;
				Spectrum f = bsdf->evaluate_sample_f(outgoing, &incident, dis(gen), dis(gen), &pdf);
				if (f.length() < 0.000001 || pdf == 0.) {
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
		Spectrum Path::calculate_radiance(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, MemoryArena& arena) const {
			return calculate_color(man, scene, ray, isect, 0, arena);
		}
	}
}
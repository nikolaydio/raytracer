#pragma once
#include "scene.h"
#include "camera.h"
#include "film.h"
#include "resource_manager.h"
#include <random>

namespace rt {
	namespace core {
		class Sampler {
			//this value indicated the generated samples per pixel. (actually sqrt(saples per pixel))
			float sampling;
		public:
			Sampler(float samples_per_pixel) : sampling(glm::sqrt(samples_per_pixel)) {}
			~Sampler() {}

			class SubSampler {
				glm::vec2 _pos, _size;
				int _max_samples;
				int _current_position;
				float _sampling;
			public:
				SubSampler(glm::vec2 pos, glm::vec2 size, float sampling);
				SubSampler() {}

				int max_samples();
				int next_samples(Sample* samples);
			};
			SubSampler create_subsampler(glm::vec2 pos, glm::vec2 size) const;
		};
	
		class Integrator {
		public:
			glm::vec3 calculate_color(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect, int depth) const {
				if (depth > 2) {
					return glm::vec3(0, 0, 0);
				}
				Material mat = man.material(isect.material);
				Spectrum emitted = mat.emitted;

				glm::vec3 BRDF = mat.reflected;
				Spectrum incident = glm::vec3(0, 0, 0);
				Intersection nested;
				Ray nray;
				Spectrum reflected(0, 0, 0);

				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> dis(-1, 1);

				for (int i = 0; i < 4; ++i) {
					nray.origin = isect.position;
					nray.direction = glm::reflect(ray.direction, isect.normal);
					nray.direction.x += dis(gen);
					nray.direction.y += dis(gen);
					nray.direction.z += dis(gen);
					nray.direction = glm::normalize(nray.direction);
					if (scene.intersect(nray, &nested)) {
						incident = calculate_color(man, scene, nray, nested, depth + 1);
						float coef = glm::abs(glm::dot(isect.normal, nray.direction));

						reflected += glm::clamp(BRDF * incident * coef, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
					}
				}
				reflected /= 4.0f;

				return glm::clamp(emitted + reflected, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
			}
			virtual Spectrum calculate_radiance(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect) const {
				return calculate_color(man, scene, ray, isect, 0);
			}
		};

		class Renderer {
			const Sampler& _sampler;
			const Camera& _camera;
			const Scene& _scene;
			const Integrator& _integrator;
			ResourceManager& _manager;
			
			Film* _film;

			void process_subsampler(Sampler::SubSampler& sampler);
		public:
			Renderer(const Sampler& sampler,
				const Camera& camera,
				const Scene& scene,
				const Integrator& integrator,
				ResourceManager& manager);

			~Renderer() {}

			Film*& film();

			void run_multithreaded(int chunk_size = 16);
			void run_singlethreaded();

			void generate_tasks() {}
		};
	}
}
#pragma once
#include "scene.h"
#include "camera.h"
#include "film.h"
#include "resource_manager.h"
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
			glm::vec3 calculate_ray() {

			}
			virtual Spectrum calculate_radiance(ResourceManager& man, const Scene& scene, Ray ray, Intersection isect) const {
				Material mat = man.material(isect.material);
				Spectrum emitted = mat.emitted;

				glm::vec3 BRDF = mat.reflected;
				Spectrum incident = glm::vec3(0, 0, 0);
				Intersection nested;
				Ray nray;
				nray.origin = isect.position;
				nray.direction = glm::normalize(glm::reflect(ray.direction, isect.normal));
				if (scene.intersect(nray, &nested)) {
					incident = man.material(nested.material).emitted;
				}
				float coef = glm::dot(isect.normal, nray.direction);

				Spectrum reflected = glm::clamp(BRDF * incident * coef, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

				return glm::clamp(emitted + reflected, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
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
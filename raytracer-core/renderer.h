#pragma once
#include "scene.h"
#include "camera.h"
#include "film.h"
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
			virtual glm::vec3 calculate_radiance(const Scene& scene, Ray ray, Intersection intersection) const {
				glm::vec3 emitted = scene.get_material(intersection.material).emitted;

				glm::vec3 BRDF = scene.get_material(intersection.material).reflected;
				glm::vec3 incident = glm::vec3(0, 0, 0);
				Intersection nested;
				Ray nray;
				nray.origin = intersection.position;
				nray.orientation = glm::reflect(ray.orientation, intersection.normal);
				if (scene.intersect(nray, &nested)) {
					incident = scene.get_material(nested.material).emitted;
				}
				float coef = 2 * glm::dot(intersection.normal, nray.orientation);

				glm::vec3 reflected = glm::max(glm::vec3(0, 0, 0), BRDF * incident * coef);
				
				coef = glm::dot(intersection.normal, -ray.orientation);
				emitted = glm::max(glm::vec3(0, 0, 0), emitted);
				for (int i = 0; i < 3; ++i) {
					
				}
				return emitted + reflected;
			}
		};

		class Renderer {
			const Sampler& _sampler;
			const Camera& _camera;
			const Scene& _scene;
			const Integrator& _integrator;
			
			Film* _film;

			void process_subsampler(Sampler::SubSampler& sampler);
		public:
			Renderer(const Sampler& sampler,
				const Camera& camera,
				const Scene& scene,
				const Integrator& integrator);

			~Renderer() {}

			Film*& film();

			void run_multithreaded(int chunk_size = 16);
			void run_singlethreaded();

			void generate_tasks() {}
		};
	}
}
#pragma once
#include "scene.h"
#include "camera.h"
#include "film.h"
namespace rt {
	namespace core {
		class Sampler {
		public:
			Sampler() {}
			~Sampler() {}

			class SubSampler {
				glm::vec2 _pos, _size;
				int _max_samples;
				int _current_position;
			public:
				SubSampler(glm::vec2 pos, glm::vec2 size);

				int max_samples();
				int next_samples(Sample* samples);
			};
			SubSampler create_subsampler(glm::vec2 pos, glm::vec2 size) const;
		};
		
		class Integrator {
		public:
			virtual glm::vec3 calculate_radiance(const Scene& scene, Intersection intersection) const {
				glm::vec3 emitted = scene.get_material(intersection.material).emitted;
				return emitted;
			}
		};

		class Renderer {
			const Sampler& _sampler;
			const Camera& _camera;
			const Scene& _scene;
			const Integrator& _integrator;
			
			Film* _film;

		public:
			Renderer(const Sampler& sampler,
				const Camera& camera,
				const Scene& scene,
				const Integrator& integrator);

			~Renderer() {}

			Film*& film();

			void run_multithreaded() {}
			void run_singlethreaded();
			void generate_tasks() {}
		};
	}
}
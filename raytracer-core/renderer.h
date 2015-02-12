#pragma once
#include "scene.h"
#include "camera.h"
#include "film.h"
#include <random>
#include "material.h"
#include "rng.h"

namespace rt {
	namespace core {
		class Sampler {
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
			virtual Spectrum calculate_radiance(const Scene& scene, Ray ray, Intersection isect, RNG& rng, MemoryArena& arena) const = 0;
		};

		class Renderer {
			const Sampler& _sampler;
			const Camera& _camera;
			const Scene& _scene;
			const Integrator& _integrator;;
			
			Film* _film;
			Film* _normals;

			bool do_not_render_flag;
			void process_subsampler(Sampler::SubSampler& sampler, MemoryArena& arena);
		public:
			Renderer(const Sampler& sampler,
				const Camera& camera,
				const Scene& scene,
				const Integrator& integrator,
				Film* radiance_film_, Film* normal_film_ = 0);

			~Renderer() {}

			Film* radiance_film();
			Film* normal_film();


			void run_multithreaded(int chunk_size = 16);
			void run_singlethreaded();

			void generate_tasks() {}
			void do_not_render();
		};
	}
}
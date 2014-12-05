#include "renderer.h"


namespace rt {
	namespace core {

		Sampler::SubSampler::SubSampler(glm::vec2 pos, glm::vec2 size, float sampling) {
			_pos = pos;
			_size = size;
			_max_samples = 64;
			_current_position = 0;
			_sampling = sampling;
		}
		int Sampler::SubSampler::max_samples() {
			return _max_samples;
		}
		int Sampler::SubSampler::next_samples(Sample* samples) {
			int completed_samples = _current_position;
			//calculate the target samples for this iteration from
			//the entire pool
			int target_samples = (int)(_size.x * _size.y * (_sampling * _sampling));
			if (target_samples > _max_samples + completed_samples){
				target_samples = _max_samples + completed_samples;
			}
			
			for (; _current_position < target_samples; ++_current_position) {
				int row = _current_position / (int)(_size.x * _sampling);
				int column = _current_position % (int)(_size.x * _sampling);

				samples[_current_position - completed_samples].position =
					_pos + glm::vec2(column / _sampling, row / _sampling);
			}
			return _current_position - completed_samples;
		}
		Sampler::SubSampler Sampler::create_subsampler(glm::vec2 pos, glm::vec2 size) const {
			return SubSampler(pos, size, sampling);
		}


		Renderer::Renderer(const Sampler& sampler,
			const Camera& camera,
			const Scene& scene,
			const Integrator& integrator,
			ResourceManager& manager)
		: _sampler(sampler), _camera(camera), _scene(scene), _integrator(integrator), _manager(manager) {

		}

		void Renderer::run_singlethreaded() {
			Sampler::SubSampler sub_sampler =
				_sampler.create_subsampler(glm::vec2(0, 0),
										_film->get_surface()->get_size());

			process_subsampler(sub_sampler);
		}
		void Renderer::process_subsampler(Sampler::SubSampler& sampler) {
			Sampler::SubSampler& sub_sampler = sampler;

			Sample* samples = new Sample[sub_sampler.max_samples()];
			while (int samples_count = sub_sampler.next_samples(samples)) {
				for (int i = 0; i < samples_count; ++i) {
					Ray ray;
					//translate samples from 0..PixelCount to 0..1
					glm::vec2 original_position = samples[i].position;
					samples[i].position /= _film->get_surface()->get_size();
					_camera.find_ray(samples[i], &ray);

					Intersection intersection;
					if (_scene.intersect(ray, &intersection)) {
						glm::vec3 color = _integrator.calculate_radiance(_manager, _scene, ray, intersection);

						_film->apply_radiance((int)original_position.x, (int)original_position.y, Color(color));
					}
				}
			}
			delete[] samples;
		}
		void Renderer::run_multithreaded(int chunk_size) {
			int size_x = _film->get_surface()->get_size().x;
			int size_y = _film->get_surface()->get_size().y;

			int columns = size_x / chunk_size + ((size_x % chunk_size == 0) ? 0 : 1);
			int rows = size_y / chunk_size + ((size_y % chunk_size == 0) ? 0 : 1);
			int chunks = columns * rows;

			//generate subsamplers
			Sampler::SubSampler* samplers = new Sampler::SubSampler[columns * rows];
			for (int i = 0; i < chunks; ++i) {
				glm::vec2 pos((i % columns) * chunk_size, (i / columns) * chunk_size);
				glm::vec2 end_pos = glm::min(glm::vec2(pos.x + chunk_size, pos.y + chunk_size), glm::vec2(size_x, size_y));
				glm::vec2 size(end_pos - pos);
				samplers[i] = _sampler.create_subsampler(pos, size);
			}

			//run the rendering process
			#pragma omp parallel for
			for (int i = 0; i < chunks; ++i) {
				process_subsampler(samplers[i]);
			}


			delete[] samplers;
		}
		Film*& Renderer::film() {
			return _film;
		}
	}
}
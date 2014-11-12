#include "renderer.h"


namespace rt {
	namespace core {

		Sampler::SubSampler::SubSampler(glm::vec2 pos, glm::vec2 size) {
			_pos = pos;
			_size = size;
			_max_samples = 64;
			_current_position = 0;
		}
		int Sampler::SubSampler::max_samples() {
			return _max_samples;
		}
		int Sampler::SubSampler::next_samples(Sample* samples) {
			int completed_samples = _current_position;
			for (; _current_position < _max_samples; ++_current_position) {
				int row = completed_samples / (int)_size.x;
				int column = completed_samples % (int)_size.x;

				samples[_current_position - completed_samples].position =
					_pos + glm::vec2(column, row);
			}
			return _current_position - completed_samples;
		}
		Sampler::SubSampler Sampler::create_subsampler(glm::vec2 pos, glm::vec2 size) const {
			return SubSampler(pos, size);
		}




		Renderer::Renderer(const Sampler& sampler,
			const Camera& camera,
			const Scene& scene,
			const Integrator& integrator) 
		: _sampler(sampler), _camera(camera), _scene(scene), _integrator(integrator) {

		}

		void Renderer::run_singlethreaded() {
			Sampler::SubSampler sub_sampler =
				_sampler.create_subsampler(glm::vec2(0, 0),
										_film->get_surface()->get_size());

			Sample* samples = new Sample[sub_sampler.max_samples()];
			while (sub_sampler.next_samples(samples)) {
				
			}
			delete[] samples;
		}
		Film*& Renderer::film() {
			return _film;
		}
	}
}
#include "renderer.h"


namespace rt {
	namespace core {
		Renderer::Renderer(const Sampler& sampler,
			const Camera& camera,
			const Scene& scene,
			const Integrator& integrator) 
		: _sampler(sampler), _camera(camera), _scene(scene), _integrator(integrator) {

		}
		void Renderer::run_singlethreaded() {

		}
		Film*& Renderer::film() {
			return _film;
		}
	}
}
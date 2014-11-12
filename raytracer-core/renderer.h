#pragma once
#include "scene.h"
#include "camera.h"
#include "film.h"
namespace rt {
	namespace core {
		class Sampler {};
		
		class Integrator {};

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
			void run_task_based() {}
		};
	}
}
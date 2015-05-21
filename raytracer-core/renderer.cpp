#include "renderer.h"

#include <iostream>
#include "material.h"
#include <omp.h>
#include <memory>
#include "config.h"
#include "rng.h"

namespace rt {
	namespace core {
		Spectrum estimate_direct(const Scene& scene, Intersection isect, const Node& light, BSDF* bsdf, glm::vec3 outgoing_w, RNG& rng, MemoryArena& arena) {
			float pdf;
			glm::vec3 light_pos = light.sample_as_light(rng.gen(), rng.gen(), rng.gen(), &pdf);
			glm::vec3 light_incident = light_pos - isect.position;
			float light_distance = glm::length(light_incident);
			light_incident = glm::normalize(light_incident);

			Intersection temp;
			Ray tray; tray.origin = isect.position; tray.direction = light_incident;
			if (!scene.intersect(tray, &temp)) {
				return Spectrum(0., 0., 0.);
			}
			if (temp.d + 0.0000001 < light_distance) {
				return Spectrum(0., 0., 0.);
			}
			return bsdf->evaluate_f(outgoing_w, light_incident) * glm::abs(glm::dot(light_incident, isect.normal)) / pdf;
		}
		Spectrum uniform_sample_one_light(const Scene& scene, Intersection isect, BSDF* bsdf, glm::vec3 outgoing_w, RNG& rng, MemoryArena& arena) {
			auto& lights = scene.get_lights();
			if (lights.size() == 0){
				return Spectrum(0., 0., 0.);
			}
			int light_index = glm::round(rng.gen() * (lights.size() - 1));
			int light_node_id = lights[light_index];
			const Node& node = scene.node(light_node_id);
			return (float)lights.size() * estimate_direct(scene, isect, node, bsdf, outgoing_w, rng, arena) * scene.material_by_id(scene.material(light_node_id)).emitted;
		}


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
			Film* radiance_film_, Film* normal_film_)
		: _sampler(sampler), _camera(camera), _scene(scene), _integrator(integrator) {
			_normals = normal_film_;
			_film = radiance_film_;
			do_not_render_flag = false;
		}
		Film* Renderer::radiance_film() {
			return _film;
		}
		Film* Renderer::normal_film() {
			return _normals;
		}

		void Renderer::run_singlethreaded() {
			Sampler::SubSampler sub_sampler =
				_sampler.create_subsampler(glm::vec2(0, 0),
										_film->get_surface()->get_size());
			MemoryArena arena(MATERIAL_MEMORY_ARENA_SIZE);
			process_subsampler(sub_sampler, arena);
		}
		void Renderer::process_subsampler(Sampler::SubSampler& sampler, MemoryArena& arena) {
			std::random_device rd;
			RNG rng(rd());
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
						Spectrum spectrum = _integrator.calculate_radiance(_scene, ray, intersection, rng, arena);
						arena.free_all();

						_film->apply_radiance((int)original_position.x, (int)original_position.y, spectrum);
						
						if (_normals) {
							_normals->apply_radiance((int)original_position.x, (int)original_position.y, glm::abs(intersection.normal));
						}
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
			std::vector<Sampler::SubSampler> samplers;
			samplers.reserve(columns*rows);
			for (int i = 0; i < chunks; ++i) {
				glm::vec2 pos((i % columns) * chunk_size, (i / columns) * chunk_size);
				glm::vec2 end_pos = glm::min(glm::vec2(pos.x + chunk_size, pos.y + chunk_size), glm::vec2(size_x, size_y));
				glm::vec2 size(end_pos - pos);
				samplers.push_back(_sampler.create_subsampler(pos, size));
			}
			std::vector<std::unique_ptr<MemoryArena>> arenas;
			int threads = omp_get_max_threads();
			for (int i = 0; i < threads; ++i) {
				arenas.push_back(std::unique_ptr<MemoryArena>(new MemoryArena(MATERIAL_MEMORY_ARENA_SIZE)));
			}

			//run the rendering process
			#pragma omp parallel for
			for (int i = 0; i < chunks; ++i) {
				if (do_not_render_flag) {
					continue;
				}
				int tid = omp_get_thread_num();
				process_subsampler(samplers[i], *arenas[tid]);
				arenas[tid]->free_all();
			}
		}

		void Renderer::do_not_render() {
			do_not_render_flag = true;
		}
	}
}

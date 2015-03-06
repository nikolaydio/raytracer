#pragma once
#include <glm/common.hpp>
#include "bsdf.h"
#include <memory>
#include "surface.h"

namespace rt {
	namespace core {
#define ARENA_NEW(arena, type) ( new (arena.allocate(sizeof(type))) (type) )
#define ARENA_NEWV(arena, type, ...) ( new (arena.allocate(sizeof(type))) (type)( __VA_ARGS__ ) )
		class MemoryArena {
		public:
			MemoryArena(int size);
			~MemoryArena();

			void* allocate(int bytes);
			void free_all();

		private:
			void* _memory;
			int _size;
			int _allocated;
		};
		class ColorFilter {
		public:
			virtual Spectrum get_color(float u, float v) = 0;
			virtual ~ColorFilter() {}
		};
		class SpectrumFilter : public ColorFilter {
		public:
			SpectrumFilter(Spectrum r) : _R(r) {}
			Spectrum _R;
			Spectrum get_color(float u, float v) {
				u; v;
				return _R;
			}
		};
		class DirectTexFilter : public ColorFilter {
			rt::core::Surface2d& _surface;
		public:
			DirectTexFilter(rt::core::Surface2d& surface) : _surface(surface) {

			}
			Spectrum get_color(float u, float v) {
				glm::vec2 size = _surface.get_size();
				//the next 2 lines are here to handle cases where uvs and <0 or abs(uv) > 1
				glm::vec2 uvs(u, v);
				uvs = uvs - glm::trunc(uvs);
				if (uvs.x < 0.f) {
					uvs.x = 1. + uvs.x;
				}
				if (uvs.y < 0.f) {
					uvs.y = 1. + uvs.y;
				}

				int xpos = (int)(size.x * uvs.x);
				int ypos = (int)(size.y * uvs.y);
				Color color = _surface.pixel(xpos, ypos);
				return color.as_vec3();
			}
		};
		class Material {
		public:
			Material() : specular(false) {}
			~Material() {}
			BSDF* get_brdf(Intersection& isect, MemoryArena& arena) const;

			std::shared_ptr<ColorFilter> diffuse;
			std::shared_ptr<ColorFilter> glossy;
			std::shared_ptr<ColorFilter> specular;


			glm::vec3 emitted;
		};
	}
}
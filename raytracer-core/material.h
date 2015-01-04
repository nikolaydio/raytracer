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
			virtual glm::vec3 get_color(float u, float v) = 0;

		};
		class SpectrumFilter : public ColorFilter {
		public:
			SpectrumFilter(Spectrum r) : R(r) {}
			Spectrum R;
			glm::vec3 get_color(float u, float v) {
				u; v;
				return R;
			}
		};
		class BilinearFilter : public ColorFilter {
			rt::core::Surface2d& _surface;
		public:
			BilinearFilter(rt::core::Surface2d& surface) : _surface(surface) {

			}
			glm::vec3 get_color(float u, float v) {
				glm::vec2 size = _surface.get_size();
				int xpos = (int)(size.x * u);
				int ypos = (int)(size.y * v);
				Color color = _surface.pixel(xpos, ypos);
				return color.as_vec3();
			}
		};
		class Material {
		public:
			Material() : specular(false) {}
			~Material() {}
			BSDF* get_brdf(Intersection& isect, MemoryArena& arena) const;

			std::shared_ptr<ColorFilter> reflected;

			glm::vec3 emitted;
			bool specular;
		};
	}
}
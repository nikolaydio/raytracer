#pragma once
#include <glm/common.hpp>
#include "bsdf.h"

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
		class Material {
		public:
			Material() : specular(false) {}
			BSDF* get_brdf(Intersection& isect, MemoryArena& arena) const;

			glm::vec3 reflected;
			glm::vec3 emitted;
			bool specular;
		};
	}
}
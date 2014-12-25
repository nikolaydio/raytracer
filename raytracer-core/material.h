#pragma once
#include <glm/common.hpp>
#include "bsdf.h"

namespace rt {
	namespace core {
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
			BSDF* get_brdf(Intersection& isect, MemoryArena& arena) const;

			glm::vec3 reflected;
			glm::vec3 emitted;
		};
	}
}
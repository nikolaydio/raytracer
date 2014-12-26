#include "material.h"
#include <iostream>
#include <new>
namespace rt {
	namespace core {
		MemoryArena::MemoryArena(int size) {
			_size = size;
			_memory = new char[size];
			_allocated = 0;
		}
		MemoryArena::~MemoryArena() {
			delete[] _memory;
			_size = 0;
			_allocated = 0;
		}

		void* MemoryArena::allocate(int bytes) {
			void* addr = ((char*)_memory) + _allocated;
			_allocated += bytes;
			if (_allocated >= _size) {
				std::cout << "Not enough memory in the memory arena. Arena size: " << _size << ", Required Total: "  << _allocated << std::endl;
				exit(0);
			}
			return addr;
		}
		void MemoryArena::free_all() {
			_allocated = 0;
		}

#define ARENA_NEW(arena, type) ( new (arena.allocate(sizeof(type))) (type) )
#define ARENA_NEW(arena, type, params) ( new (arena.allocate(sizeof(type))) (type)(params) )

		BSDF* Material::get_brdf(Intersection& isect, MemoryArena& arena) const {
			BSDF* bsdf = ARENA_NEW(arena, BSDF, isect.normal);
			bsdf->add_brdf(ARENA_NEW(arena, LambertianBRDF, Spectrum(reflected)), 1);
			//bsdf->add_brdf(ARENA_NEW(arena, SpecularReflectionBRDF, Spectrum(1,1,1)), 0.2);
			return bsdf;
		}
	}
}
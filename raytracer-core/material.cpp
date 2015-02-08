#include "material.h"
#include "microfacet.h"

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


		BSDF* Material::get_brdf(Intersection& isect, MemoryArena& arena) const {
			BSDF* bsdf = ARENA_NEWV(arena, BSDF, isect.normal);
			if (diffuse) {
				bsdf->add_brdf(ARENA_NEWV(arena, LambertianBRDF, diffuse->get_color(isect.uv.x, isect.uv.y)), 1);
			}
			if (specular)  {
				bsdf->add_brdf(ARENA_NEWV(arena, SpecularReflectionBRDF, specular->get_color(isect.uv.x, isect.uv.y)), 1);
			}
			if (glossy) {
				//The blinn exponent should be editable from config file in the future
				MicrofacetDistribution* distr = ARENA_NEWV(arena, Blinn, 360.f);
				BRDF* spec = ARENA_NEWV(arena, Microfacet, glossy->get_color(isect.uv.x, isect.uv.y), distr);
				bsdf->add_brdf(spec, 1.0f);
			}
			
			return bsdf;
		}
	}
}
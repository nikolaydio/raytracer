#pragma once
#include "surface.h"

namespace rt {
	namespace core {

		class Film {
			Surface2d* _surface;
			int* samples;
		public:
			void apply_radiance(int x, int y, Spectrum value);
			Surface2d* get_surface();

			//The surface2d is destroyed, when the film is.
			Film(Surface2d* target);
			~Film();
		};
	}
}
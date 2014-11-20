#pragma once
#include "surface.h"

namespace rt {
	namespace core {

		class Film {
			Surface2d* _surface;
			int* samples;
		public:
			void apply_radiance(int x, int y, Color value);
			Surface2d* get_surface();

			Film(Surface2d* target);
			~Film();
		};
	}
}
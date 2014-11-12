#pragma once
#include "surface.h"

namespace rt {
	namespace core {

		class Film {
			Surface2d* _surface;
		public:
			void apply_radiance(int x, int y, Color value);

			Film(Surface2d* target);
			~Film();
		};
	}
}
#pragma once
#include "surface.h"

namespace rt {
	namespace core {

		class Film {
			Surface2d* _surface;
		public:

			Film(Surface2d* target);
			~Film();
		};
	}
}
#pragma once

#include <vector>
#include "surface.h"

namespace rt {
	namespace sdl {
		void present_rendering(std::vector<rt::core::Surface2d*>& surfaces);
	}
}
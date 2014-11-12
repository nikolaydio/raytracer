#pragma once
#include <glm/glm.hpp>

namespace rt {
	namespace core {
		//assume R8G8B8X8 as default
		//support for others may be implemented in the future
		struct Color {
			Color(unsigned char pr, unsigned char pg, unsigned char pb, unsigned char px) {
				r = pr;
				g = pg;
				b = pb;
				x = px;
			}
			Color() {
				r = 255; g = 255;
				b = 255; x = 255;
			}
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char x;
		};
		class Surface2d {
			int _width, _height;
			Color* _pixels;
		public:

			Surface2d(int width, int height);
			~Surface2d();

			Color& pixel(int x, int y);
			glm::vec2 get_size();
		};
	}
}
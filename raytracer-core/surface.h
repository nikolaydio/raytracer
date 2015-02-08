#pragma once
#include <glm/glm.hpp>
#include "types.h"

namespace rt {
	namespace core {
		//assume R8G8B8 as default
		//support for others may be implemented in the future
#pragma pack(push, 1)
		struct Color {
			Color(unsigned char pr, unsigned char pg, unsigned char pb, unsigned char px) {
				r = pr;
				g = pg;
				b = pb;
				x = px;
			}
			Color() {
				r = 0; g = 0;
				b = 0; x = 255;
			}
			explicit Color(Spectrum color) {
				color = glm::clamp(color, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

				r = (unsigned char)(color.x * 255.f);
				g = (unsigned char)(color.y * 255.f);
				b = (unsigned char)(color.z * 255.f);
				x = 255;
			}
			glm::vec3 as_vec3() {
				glm::vec3 res;
				res.x = (float)r / 255.0f;
				res.y = (float)g / 255.0f;
				res.z = (float)b / 255.0f;
				return res;
			}
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char x;
		};
#pragma pack(pop)
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
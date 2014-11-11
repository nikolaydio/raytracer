#include "surface.h"

namespace rt {
	namespace core {


		Surface2d::Surface2d(int width, int height) {
			_pixels = new Color[width * height];
			_width = width;
			_height = height;
		}
		Surface2d::~Surface2d() {
			delete[] _pixels;
		}

		Color& Surface2d::pixel(int x, int y) {
			return _pixels[y * _width + x];
		}
	}
}
#include "camera.h"


namespace rt {
	namespace core {
		Film::Film(int width, int height) {
			_pixels = new Pixel[width * height];
			_width = width;
			_height = height;
		}
		Film::~Film() {
			delete[] _pixels;
		}

		Pixel& Film::pixel(int x, int y) {
			return _pixels[y * _width + x];
		}
	}
}
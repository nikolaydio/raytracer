#include "camera.h"
#include "film.h"

namespace rt {
	namespace core {
		Film::Film(Surface2d* surface) : _surface(surface) {


		}
		Film::~Film() {

		}
		void Film::apply_radiance(int x, int y, Color value) {
			_surface->pixel(x, y) = value;
		}
		Surface2d* Film::get_surface() {
			return _surface;
		}
	}
}
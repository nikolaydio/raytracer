#include "camera.h"
#include "film.h"

namespace rt {
	namespace core {
		Film::Film(Surface2d* surface) : _surface(surface) {


		}
		Film::~Film() {

		}
		Surface2d* Film::get_surface() {
			return _surface;
		}
	}
}
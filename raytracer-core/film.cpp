#include "camera.h"
#include "film.h"

namespace rt {
	namespace core {
		Film::Film(Surface2d* surface) : _surface(surface) {
			int count = (int)surface->get_size().x * (int)surface->get_size().y;
			samples = new int[count];
			for (int i = 0; i < count; ++i) {
				samples[i] = 0;
			}
		}
		Film::~Film() {
			delete[] samples;
		}
		void Film::apply_radiance(int x, int y, Color value) {
			int width = (int)_surface->get_size().x;
			unsigned int samples_here = samples[y * width + x];
			unsigned int X = _surface->pixel(x, y).r * samples_here;
			unsigned int Y = _surface->pixel(x, y).g * samples_here;
			unsigned int Z = _surface->pixel(x, y).b * samples_here;
			X += value.r;
			Y += value.g;
			Z += value.b;
			samples_here += 1;
			X /= samples_here;
			Y /= samples_here;
			Z /= samples_here;
			samples[y * width + x] = samples_here;
			_surface->pixel(x, y) = Color(X,Y,Z, 255);
		}
		Surface2d* Film::get_surface() {
			return _surface;
		}
	}
}
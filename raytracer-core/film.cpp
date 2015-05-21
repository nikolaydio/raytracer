#include "camera.h"
#include "film.h"

namespace rt {
	namespace core {
		Film::Film(Surface2d* surface) : _surface(surface) {
			int count = (int)surface->get_size().x * (int)surface->get_size().y;
			samples = new int[count];
			color_matrix = new Spectrum[count];
			for (int i = 0; i < count; ++i) {
				samples[i] = 0;
			}
		}
		Film::~Film() {
			delete[] samples;
			delete _surface;
			delete[] color_matrix;
		}
		void Film::apply_radiance(int x, int y, Spectrum value) {
			int width = (int)_surface->get_size().x;
			color_matrix[y * width + x] += value;
			samples[y * width + x] += 1;
			unsigned int samples_here = samples[y * width + x];
			_surface->pixel(x, y) = Color((color_matrix[y * width + x] / (float)samples_here));
		}
		Surface2d* Film::get_surface() {
			return _surface;
		}
	}
}
#pragma once

namespace rt {
	namespace core {
		struct Pixel {
			Pixel(char _r, char _g, char _b, char _x) {
				r = _r;
				g = _g;
				b = _b;
				x = _x;
			}
			Pixel() {
				x = 0; r = 255; g = 255; b = 255;
			}
			char r;
			char g;
			char b;
			char x;
		};
		//Film contains the target image. Default format is R8G8B8X8
		class Film {
			int _width, _height;
			Pixel* _pixels;
		public:

			Film(int width, int height);
			~Film();

			Pixel& pixel(int x, int y);
		};
		class Camera {
		public:
			Camera() {}
			~Camera() {}

			
		};
	}
}
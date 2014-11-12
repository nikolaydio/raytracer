#pragma once
#include <glm/glm.hpp>

#include "scene.h"

namespace rt {
	namespace core {
		class Camera {
		public:
			Camera() {}
			~Camera() {}

			void GenerateRay(Sample sample, Ray* ray);
		private:
			glm::mat4x4 transform;
		};
	}
}
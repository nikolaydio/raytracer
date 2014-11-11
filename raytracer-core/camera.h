#pragma once
#include <glm/glm.hpp>

namespace rt {
	namespace core {
		class Camera {
		public:
			Camera() {}
			~Camera() {}


		private:
			glm::mat4x4 transform;
		};
	}
}
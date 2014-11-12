#pragma once
#include <glm/glm.hpp>

#include "scene.h"

namespace rt {
	namespace core {
		class Camera {
		public:
			Camera(glm::mat4x4 cam_to_world, float fov, float aspect_ratio);
			~Camera() {}

			void find_ray(Sample sample, Ray* ray) const;
		private:
			glm::mat4x4 _camera_to_world;
			glm::vec2 _field_of_view;
		};
	}
}
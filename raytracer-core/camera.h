#pragma once
#include <glm/glm.hpp>

#include "scene.h"

namespace rt {
	namespace core {
		glm::mat4 create_camera_to_world(glm::vec3 eye, glm::vec3 target);
		class Camera {
		public:
			Camera(glm::vec3 eye, glm::vec3 target, float fov, float aspect_ratio);
			~Camera() {}

			void find_ray(Sample sample, Ray* ray) const;
		private:
			glm::mat4x4 _camera_to_world;
			glm::vec2 _field_of_view;
		};
	}
}
#include "camera.h"
#include <glm/gtc/constants.hpp>

namespace rt {
	namespace core {
		float to_radians(float degree) {
			return degree * glm::pi<float>() / 180;
		}
		Camera::Camera(glm::mat4x4 cam_to_world, float fov, float aspect_ratio) : _camera_to_world(cam_to_world) {
			//we are interested in their halves;
			fov /= 2;
			float yfov = fov / aspect_ratio;

			float x_coeff = 1 / glm::sin(to_radians(90 - fov));
			float y_coeff = 1 / glm::sin(to_radians(90 - yfov));

			_field_of_view.x = x_coeff * glm::sin(to_radians(fov));
			_field_of_view.y = y_coeff * glm::sin(to_radians(yfov));
			//this operation would look better in the find_ray funtion
			//but here it is calculated just once.
			_field_of_view *= 2.0f;
		}
		void Camera::find_ray(Sample sample, Ray* ray) const {
			ray->origin = glm::vec3(0, 0, 0);
			glm::vec2 orientation = sample.position - glm::vec2(0.5, 0.5);
			orientation *= _field_of_view;
			ray->orientation = glm::normalize(glm::vec3(orientation, 1));

			//ray->origin = xyz(_camera_to_world * glm::vec4(ray->origin, 1));
			//ray->orientation = glm::normalize(xyz(_camera_to_world * glm::vec4(ray->orientation, 0)));
		}
	}
}
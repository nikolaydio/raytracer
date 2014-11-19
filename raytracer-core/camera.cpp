#include "camera.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

namespace rt {
	namespace core {
		float to_radians(float degree) {
			return degree * glm::pi<float>() / 180;
		}
		glm::mat4 create_camera_to_world(glm::vec3 eye, glm::vec3 target) {
			glm::mat4 matrix(1.0f);
			matrix = glm::lookAt(eye, target, glm::vec3(0, 1, 0));
			matrix = glm::inverse(matrix);

			return matrix;
		}
		Camera::Camera(glm::vec3 eye, glm::vec3 target, float fov, float aspect_ratio) {
			_camera_to_world = create_camera_to_world(eye, target);

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

			ray->origin = glm::vec3(_camera_to_world * glm::vec4(ray->origin, 1));
			ray->orientation = -glm::vec3(_camera_to_world * glm::vec4(ray->orientation, 0));
		}
	}
}
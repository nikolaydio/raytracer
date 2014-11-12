#include "camera.h"
#include <glm/gtc/constants.hpp>

namespace rt {
	namespace core {
		glm::vec3 xyz(const glm::vec4 _this) {
			return glm::vec3(_this.x, _this.y, _this.z);
		}
		Camera::Camera(glm::mat4x4 cam_to_world, float fov, float aspect_ratio) : _camera_to_world(cam_to_world) {
			//we are interested in their halves;
			fov /= 2;
			float yfov = fov / aspect_ratio;

			float x_coeff = 1 / glm::sin(fov * glm::pi<float>() / 180.f);
			float y_coeff = 1 / glm::sin(yfov * glm::pi<float>() / 180.f);

			_field_of_view.x = x_coeff * glm::sin((90 - fov) * glm::pi<float>() / 180.f);
			_field_of_view.y = y_coeff * glm::sin((90 - yfov) * glm::pi<float>() / 180.f);
			
		}
		void Camera::find_ray(Sample sample, Ray* ray) const {
			ray->origin = glm::vec3(0, 0, 0);
			glm::vec2 orientation = sample.position - glm::vec2(0.5, 0.5);
			orientation *= _field_of_view;
			ray->orientation = glm::vec3(sample.position, 1);

			ray->origin = xyz(_camera_to_world * glm::vec4(ray->origin, 1));
			ray->orientation = xyz(_camera_to_world * glm::vec4(ray->orientation, 0));
		}
	}
}
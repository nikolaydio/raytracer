#pragma once
#include <glm/common.hpp>

namespace rt {
	namespace core {
		struct Ray{
			glm::vec3 origin;
			glm::vec3 orientation;
		};
		struct Material;
		struct Intersection{
			//D is the distance from the ray origin to the intersection point.
			//most algorithms calculate this first and it is easier to do some other checks with it
			float d;
			glm::vec3 position;
			glm::vec3 normal;
			Material* material;
		};
		struct Material {
			glm::vec3 color;
		};
		struct Sample {
			glm::vec2 position;
		};
		class Shape {
		public:
			virtual bool intersect(Ray ray, Intersection* result) const = 0;
		};

	}
}
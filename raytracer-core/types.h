#pragma once
#include <glm/common.hpp>

namespace rt {
	namespace core {
		typedef uint32_t MaterialId;

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
			MaterialId material;
		};
		struct Material {
			glm::vec3 reflected;
			glm::vec3 emitted;
		};
		struct Sample {
			glm::vec2 position;
		};

		//those 2 classes should not be here!!
		class AABB {
		public:
			AABB(const glm::vec3 p) {
				_min = p;
				_max = p;
			}
			AABB(const glm::vec3 p1, const glm::vec3 p2) {
				//not sure if this work as intended
				_min = glm::min(p1, p2);
				_max = glm::max(p1, p2);
			}
			AABB(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 p3) {
				//not sure if this work as intended
				_min = glm::min(glm::min(p1, p2), p3);
				_max = glm::max(glm::max(p1, p2), p3);
			}
			AABB union_point(const glm::vec3 p) {
				return AABB(_min, _max, p);
			}

			glm::vec3 _min;
			glm::vec3 _max;

		};
		class Shape {
		public:
			virtual bool intersect(Ray ray, Intersection* result) const = 0;
			virtual AABB get_bounding_box() const = 0;
		};

	}
}
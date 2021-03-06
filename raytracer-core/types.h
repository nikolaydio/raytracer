#pragma once
#include <glm/common.hpp>

namespace rt {
	namespace core {
		typedef uint32_t MaterialId;
		unsigned int MurmurHash2(const void * key, int len, unsigned int seed);

		struct Ray{
			glm::vec3 origin;
			glm::vec3 direction;
		};
		struct Intersection{
			//D is the distance from the ray origin to the intersection point.
			//most algorithms calculate this first and it is easier to do some other checks with it
			Intersection() {
				d = 0;
				position = glm::vec3(0, 0, 0);
				normal = glm::vec3(0,0, 0);
				uv = glm::vec2(0, 0);
				material = -1;
			}
			float d;
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			MaterialId material;
		};

		struct Sample {
			glm::vec2 position;
		};
		typedef glm::vec3 Spectrum;

		//those 2 classes should not be here!!
		class AABB {
		public:
			AABB() {
			}
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
			AABB(const AABB a, const AABB b) {
				_min = glm::min(a._min, b._min);
				_max = glm::max(a._max, b._max);
			}
			AABB union_point(const glm::vec3 p) {
				return AABB(_min, _max, p);
			}
			bool intersect(Ray ray) const {
				glm::vec3 dirfrac = 1.0f / ray.direction;

				float t1 = (_min.x - ray.origin.x)*dirfrac.x;
				float t2 = (_max.x - ray.origin.x)*dirfrac.x;
				float t3 = (_min.y - ray.origin.y)*dirfrac.y;
				float t4 = (_max.y - ray.origin.y)*dirfrac.y;
				float t5 = (_min.z - ray.origin.z)*dirfrac.z;
				float t6 = (_max.z - ray.origin.z)*dirfrac.z;

				float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
				float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

				if (tmax < 0) {
					//t = tmax;
					return false;
				}
				if (tmin > tmax){
					//t = tmax;
					return false;
				}
				//t = tmin;
				return true;
			}
			float intersect_t(Ray ray) const {
				glm::vec3 dirfrac = 1.0f / ray.direction;

				float t1 = (_min.x - ray.origin.x)*dirfrac.x;
				float t2 = (_max.x - ray.origin.x)*dirfrac.x;
				float t3 = (_min.y - ray.origin.y)*dirfrac.y;
				float t4 = (_max.y - ray.origin.y)*dirfrac.y;
				float t5 = (_min.z - ray.origin.z)*dirfrac.z;
				float t6 = (_max.z - ray.origin.z)*dirfrac.z;

				float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
				float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

				if (tmax < 0) {
					return 0.f;
				}
				if (tmin > tmax){
					return 0.f;
				}
				return tmin;
			}
			bool intersect(const AABB b) const {
				if (_max.x < b._min.x) {
					return false;
				}
				if (_max.y < b._min.y) {
					return false;
				}
				if (_max.z < b._min.z) {
					return false;
				}
				if (_min.x > b._max.x) {
					return false;
				}
				if (_min.y > b._max.y) {
					return false;
				}
				if (_min.z > b._max.z) {
					return false;
				}
				return true;
			}
			glm::vec3 _min;
			glm::vec3 _max;

		};

	}
}
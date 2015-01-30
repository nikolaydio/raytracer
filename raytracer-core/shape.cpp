#include "shape.h"
#include <glm/glm.hpp>
#include "accelerator.h"
#include <vector>

namespace rt {
	namespace core {
		glm::vec3 spherical_direction(float sintheta, float costheta, float phi,
			const glm::vec3 x, const glm::vec3 y, const glm::vec3 z) {
			return sintheta * glm::cos(phi) * x + sintheta * glm::sin(phi) * y + costheta * z;
		}
		glm::vec3 spherical_direction(float sintheta, float costheta, float phi) {
			return glm::vec3(sintheta * glm::cos(phi),
				sintheta * glm::sin(phi),
				costheta);
		}
		glm::vec2 spherical_angles(const glm::vec3 dir) {
			return glm::vec2(glm::acos(dir.z), glm::atan(dir.y / dir.z));
		}
		glm::vec3 spherical_angles_tri(const glm::vec3 dir) {
			return glm::vec3(0, 0, 0);
		}


		static bool IntersectSphere(glm::vec3 sphere_pos, float radius, glm::vec3 rpos, glm::vec3 rdir, float* dist_out) {
			glm::vec3 L = sphere_pos - rpos;
			float tca = glm::dot(L, rdir);
			if (tca < 0) return false;
			float d2 = glm::dot(L, L) - tca * tca;
			if (d2 > radius) return false;
			float thc = glm::sqrt(radius - d2);
			float t0 = tca - thc;
			float t1 = tca + thc;


			if (t0 > t1) {
				float temp = t0;
				t0 = t1;
				t1 = temp;
			}

			if (t1 < 0)
				return false;

			if (t0 < 0){
				*dist_out = t1;
				return true;
			}
			else{
				*dist_out = t0;
				return true;
			}
		}
		bool Sphere::intersect(Ray ray, Intersection* result) const {
			if (IntersectSphere(_pos, _radius, ray.origin, ray.direction, &result->d)) {
				result->position = (ray.origin + ray.direction * result->d);
				result->normal = glm::normalize(result->position - _pos);
				return true;
			}
			return false;
		}
		AABB Sphere::get_bounding_box() const {
			return AABB(_pos - _radius, _pos + _radius);
		}

		bool Triangle::intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec3 s1 = glm::cross(ray.direction, e2);
			float divisor = glm::dot(s1, e1);

			if (glm::abs(divisor) < 0.00000001) {
				return false;
			}
			float invDivisor = 1.f / divisor;
			//first barycentric coordinate
			glm::vec3 d = ray.origin - p0;
			float b1 = glm::dot(d, s1) * invDivisor;
			if (b1 < 0. || b1 > 1.) {
				return false;
			}
			//second
			glm::vec3 s2 = glm::cross(d, e1);
			float b2 = glm::dot(ray.direction, s2) * invDivisor;
			if (b2 <0. || b1 + b2 > 1.) {
				return false;
			}
			float t = glm::dot(e2, s2) * invDivisor;

			//this is a threashhold for not colliding with itself
			if (t < 0.0001) {
				return false;
			}
			result->d = t;
			result->position = ray.origin + ray.direction * t;
			result->normal = glm::normalize(glm::cross(e1, e2));
			float b0 = 1.f - b1 - b2;
			result->uv = b0 * uv0 + b1 * uv1 + b2 * uv2;
			assert(result->uv.x >= 0 && result->uv.x <= 1);
			assert(result->uv.y >= 0 && result->uv.y <= 1);
			return true;
		}
		rt::core::AABB Triangle::get_bounding_box() const {
			return rt::core::AABB(p0, p1, p2);
		}
		glm::vec3 Triangle::sample(float u1, float u2, float u3) const {
			return (1.f - glm::sqrt(u1)) * p0 + (glm::sqrt(u1) * (1 - u2)) * p1 + (glm::sqrt(u1) * u2) * p2;
		}




		int Mesh::MeshAdapter::count() {
					return _points.size() / 3;
				}
		AABB Mesh::MeshAdapter::get_bounding_box(int index) const {
			rt::core::AABB aabb(
				_points[index * 3], _points[index * 3 + 1], _points[index * 3 + 2]);

			return aabb;
		}
		bool Mesh::MeshAdapter::intersect(int index, rt::core::Ray ray, rt::core::Intersection* result) const {
			glm::vec2 uvs[3];
			if (_uvs.size() == _points.size()) {
				uvs[0] = _uvs[index * 3];
				uvs[1] = _uvs[index * 3 + 1];
				uvs[2] = _uvs[index * 3 + 2];
			}
			else{
				uvs[0] = glm::vec2(0, 0);
				uvs[1] = glm::vec2(1, 0);
				uvs[2] = glm::vec2(0, 1);
			}
			Triangle tri(_points[index * 3], _points[index * 3 + 1], _points[index * 3 + 2],
				uvs[0], uvs[1], uvs[2]);
			return tri.intersect(ray, result);
		}


		ElementAdapter& Mesh::get_adapter() {
			return _adapter;
		}
		void Mesh::push_face(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
			points.push_back(a);
			points.push_back(b);
			points.push_back(c);
		}
		void Mesh::push_vert(glm::vec3 a) {
			points.push_back(a);
		}
		void Mesh::push_vert(glm::vec3 a, glm::vec2 uv) {
			points.push_back(a);
			_uvs.push_back(uv);
		}
		void Mesh::set_accelerator(rt::core::Accelerator* acc) {
			_accelerator = acc;
			acc->rebuild(_adapter);
			bbox = get_bounding_box();
		}
		bool Mesh::intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
			if (!bbox.intersect(ray)) {
				return false;
			}
			assert(_accelerator);
			return _accelerator->intersect(ray, result);
		}
		AABB Mesh::get_bounding_box() const {
			AABB aabb(points[0]);
			for (int i = 1; i < points.size(); ++i) {
				aabb = aabb.union_point(points[i]);
			}
			return aabb;
		}

		glm::vec3 Mesh::sample(float u1, float u2, float u3) const {
			//choose face randomly.
			//better approach would be to use comulative distr func
			int index = glm::round(u1 * ((points.size() / 3) - 1));
			glm::vec2 uvs[3];
			Triangle tri(points[index * 3], points[index * 3 + 1], points[index * 3 + 2],
				uvs[0], uvs[1], uvs[2]);
			return tri.sample(u2, u3, 0);
		}
	}
}
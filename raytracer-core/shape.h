#pragma once
#include "types.h"
#include <vector>
#include "accelerator.h"

namespace rt {
	namespace core {

		//2 utility functions
		glm::vec3 spherical_direction(float sintheta, float costheta, float phi,
			const glm::vec3 x, const glm::vec3 y, const glm::vec3 z);
		glm::vec3 spherical_direction(float sintheta, float costheta, float phi);

		//to thetha(x) and phi(y)
		glm::vec2 spherical_angles(const glm::vec3 dir);
		//to sintheta(x), costheta(y) and phi(z)
		glm::vec3 spherical_angles_tri(const glm::vec3 dir);

		class Shape {
		public:
			virtual bool intersect(Ray ray, Intersection* result) const = 0;
			virtual AABB get_bounding_box() const = 0;
		};


		class Sphere : public Shape {
			glm::vec3 _pos;
			float _radius;
		public:
			Sphere(glm::vec3 pos, float radius)
				: _pos(pos), _radius(radius) {}
			bool intersect(Ray ray, Intersection* result) const;
			AABB get_bounding_box() const;
		};

		class Triangle : public Shape {
			glm::vec3 p0, p1, p2;
		public:
			Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) : p0(a), p1(b), p2(c) {}

			bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const;
			rt::core::AABB get_bounding_box() const;
		};


		class Mesh : public Shape {
			std::vector<glm::vec3> points;
			class MeshAdapter : public ElementAdapter {
				std::vector<glm::vec3>& _mesh;

			public:
				MeshAdapter(std::vector<glm::vec3>& mesh) : _mesh(mesh) {}
				virtual int count();
				virtual rt::core::AABB get_bounding_box(int index) const;
				virtual bool intersect(int index, rt::core::Ray ray, rt::core::Intersection* result) const;
			};
			MeshAdapter _adapter;
			Accelerator* _accelerator;
			AABB bbox;
		public:
			Mesh() : _accelerator(0), _adapter(points) { }
			~Mesh() { delete _accelerator; }
			ElementAdapter& get_adapter();
			void push_face(glm::vec3 a, glm::vec3 b, glm::vec3 c);
			void push_vert(glm::vec3 a);
			void set_accelerator(rt::core::Accelerator* acc);
			bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const;
			AABB get_bounding_box() const;
		};
	}
}
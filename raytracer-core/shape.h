#pragma once
#include "types.h"
#include <vector>
#include "accelerator.h"

namespace rt {
	namespace core {
		class Shape {
		public:
			virtual bool intersect(Ray ray, Intersection* result) const = 0;
			virtual AABB get_bounding_box() const = 0;
			virtual ~Shape() {}
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
			glm::vec2 uv0, uv1, uv2;
		public:
			Triangle(const glm::vec3 a, const glm::vec3 b, const glm::vec3 c,
				const glm::vec2 _uv0, const glm::vec2 _uv1, const glm::vec2 _uv2) {
				p0 = a;
				p1 = b;
				p2 = c;
				uv0 = _uv0;
				uv1 = _uv1;
				uv2 = _uv2;
			}

			bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const;
			rt::core::AABB get_bounding_box() const;
		};


		class Mesh : public Shape {
			std::vector<glm::vec3> points;
			std::vector<glm::vec2> _uvs;
			class MeshAdapter : public ElementAdapter {
				std::vector<glm::vec3>& _points;
				std::vector<glm::vec2>& _uvs;
			public:
				MeshAdapter(std::vector<glm::vec3>& points, std::vector<glm::vec2>& uvs) : _points(points),
				_uvs(uvs) {}
				virtual int count();
				virtual rt::core::AABB get_bounding_box(int index) const;
				virtual bool intersect(int index, rt::core::Ray ray, rt::core::Intersection* result) const;
			};
			MeshAdapter _adapter;
			Accelerator* _accelerator;
			AABB bbox;
		public:
			Mesh() : _accelerator(0), _adapter(points, _uvs) { }
			~Mesh() { delete _accelerator; }
			ElementAdapter& get_adapter();
			void push_face(glm::vec3 a, glm::vec3 b, glm::vec3 c);
			void push_vert(glm::vec3 a);
			void push_vert(glm::vec3 a, glm::vec2 uv);
			void set_accelerator(rt::core::Accelerator* acc);
			bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const;
			AABB get_bounding_box() const;
		};
	}
}
#pragma once
#include <vector>
#include <glm\glm.hpp>

namespace rt {
	namespace core {
		class Material {
		public:
			glm::vec3 color;
		};


		class Intersection{
		public:
			//D is the distance from the ray origin to the intersection point.
			//most algorithms calculate this first and it is easier to do some other checks with it
			float d;
			glm::vec3 position;
			glm::vec3 normal;
			Material* material;
		};
		class Ray{
		public:
			glm::vec3 origin;
			glm::vec3 orientation;
		};
		class Shape {
		public:
			virtual bool intersect(glm::mat4 transform, Ray ray, Intersection* result) const = 0;
		};
		class Primitive{
			glm::mat4 _transform;
			Material* _material;
			Shape* _shape;
		public:
			Primitive(glm::mat4 transform, Material* mat, Shape* shape) :
				_transform(transform), _material(mat), _shape(shape) {}
			~Primitive() {}

			virtual bool intersect(Ray ray, Intersection* result) const;
		};
		class Sample {
		public:
			glm::vec2 position;
		};
		//Keeps the list of objects and performs the ray tracing
		//The idea is to allow it to do some of spatial paritioning.
		class Scene {
		public:
			Scene() {}
			~Scene() {}

			bool intersect(Ray ray, Intersection* result) const;
			void add_primitive(Primitive* prim);
		private:
			std::vector<Primitive*> _primitives;
			//shared scene global materials and shapes.
			std::vector<Shape*> _shapes;
			std::vector<Material*> _materials;
		};
	}
}
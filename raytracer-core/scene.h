#pragma once
#include <vector>
#include <glm\glm.hpp>

namespace rt {
	namespace core {
		class Material {
		public:
			glm::vec3 color;
		};

		//Keeps the list of objects and performs the ray tracing
		//The idea is to allow it to do some of spatial paritioning.
		class Intersection{
		public:
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
			virtual bool intersect(Ray ray, Intersection* result) const = 0;
		};
		class Primitive{
		public:
			Primitive() {}
			~Primitive() {}

			virtual bool intersect(Ray ray, Intersection* result) const = 0;
		};
		class Sample {
		public:
			glm::vec2 position;
		};
		class Scene {
		public:
			Scene() {}
			~Scene() {}

			bool intersect(Ray ray, Intersection* result) const;
		private:
			std::vector<Primitive*> _primitives;
			//shared scene global materials and shapes.
			std::vector<Shape*> _shapes;
			std::vector<Material*> _materials;
		};
	}
}
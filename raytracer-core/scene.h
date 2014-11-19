#pragma once
#include <vector>
#include <glm\glm.hpp>
#include "primitive.h"
#include "types.h"

namespace rt {
	namespace core {

		class Scene;
		class Accelerator {
		public:
			virtual bool intersect(Ray ray, Intersection* result) const = 0;
			virtual void build_from(const Scene& scene) = 0;
		};

		class DefaultAccelerator : public Accelerator {
		public:
			virtual bool intersect(Ray ray, Intersection* result) const;
			virtual void build_from(const Scene& scene);
		private:
			Scene const * _scene;
		};


		class Scene {
		public:
			Scene() : _accelerator(0) {}
			~Scene() {}

			bool intersect(Ray ray, Intersection* result) const;
			void add_primitive(Primitive* prim);

			const Material& get_material(MaterialId id) const;
			MaterialId new_material(Material mat);

			int primitive_count() const;
			const Primitive* get_primitive(int index) const;

			void accelerate_and_rebuild(Accelerator* pacc);
		private:
			std::vector<Primitive*> _primitives;
			std::vector<Material> _materials;

			Accelerator* _accelerator;
		};
	}
}
#include "scene.h"

namespace rt {
	namespace core {

		bool Scene::intersect(Ray ray, Intersection* result) const {
			return aggregate->intersect(ray, result);
		}

		void Scene::set_primitive(Primitive* prim) {
			aggregate = prim;
		}
	}
}
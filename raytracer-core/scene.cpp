#include "scene.h"

namespace rt {
	namespace core {


		bool Primitive::intersect(Ray ray, Intersection* result) const {
			if (_shape->intersect(_transform, ray, result)) {
				result->material = _material;
				return true;
			}
			return false;
		}



		bool Scene::intersect(Ray ray, Intersection* result) const {
			Intersection check;
			check.d = 0;

			//This function will change drastically in next iterations
			//I really can't think of better solution here without introducing a temp variable

			result->d = 9999999999;
			for (auto i = _primitives.begin(), end = _primitives.end();
				i != end; ++i) {
				if ((*i)->intersect(ray, &check)) {
					if (check.d < result->d) {
						*result = check;
					}
				}

			}
			if (result->d > 999999999) {
				return false;
			}
			return true;
		}

		void Scene::add_primitive(Primitive* prim) {
			_primitives.push_back(prim);
		}
	}
}
#include "scene.h"

namespace rt {
	namespace core {

		bool DefaultAccelerator::intersect(Ray ray, Intersection* result) const {
			Intersection check;
			check.d = 0;

			result->d = 9999999999;
			for (int i = 0; i < _scene->primitive_count(); ++i) {
				if (_scene->get_primitive(i)->intersect(ray, &check)) {
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
		void DefaultAccelerator::build_from(const Scene& scene) {
			_scene = &scene;
		}




		bool Scene::intersect(Ray ray, Intersection* result) const {
			assert(_accelerator);
			return _accelerator->intersect(ray, result);
		}

		void Scene::add_primitive(Primitive* prim) {
			assert(prim);
			_primitives.push_back(prim);
		}

		const Material& Scene::get_material(MaterialId id) const {
			return _materials[id];
		}
		MaterialId Scene::new_material(Material mat) {
			_materials.push_back(mat);
			return _materials.size() - 1;
		}

		int Scene::primitive_count() const {
			return _primitives.size();
		}
		const Primitive* Scene::get_primitive(int index) const {
			return _primitives[index];
		}

		void Scene::accelerate_and_rebuild(Accelerator* pacc) {
			//was thinking to add a case where the same object is passed
			//to rebuild the scene, but it is too confusing

			assert(pacc != _accelerator);
			if (pacc != 0) {
				delete _accelerator;
				pacc->build_from(*this);
				_accelerator = pacc;
			} else if (_accelerator != 0) {
				_accelerator->build_from(*this);
			} else {
				//old is 0, new is 0. Probably a mistake. Better tell the human.
				assert(0);
			}
		}
	}
}
#include "scene.h"

namespace rt {
	namespace core {



		bool Scene::intersect(Ray ray, Intersection* result) const {
			assert(_accelerator);
			return _accelerator->intersect(ray, result);
		}

		uint32_t Scene::node_count() const {
			return _node_count;
		}
		uint32_t Scene::node_capacity() const {
			return _node_capacity;
		}
		uint32_t Scene::set_capacity(uint32_t new_total) {
			Node* new_nodes = 0;
			MaterialId* new_mats = 0;
			int to_copy = glm::min(_node_count, new_total);
			_node_count = to_copy;

			if (new_total != 0) {
				new_nodes = new Node[new_total];
				new_mats = new MaterialId[new_total];
			}
			
			if (_nodes != 0) {
				memcpy(new_nodes, _nodes, sizeof(Node) * to_copy);
				delete[] _nodes;
			}
			if (_materials != 0) {
				memcpy(new_mats, _materials, sizeof(MaterialId) * to_copy);
				delete[] _materials;
			}
			_nodes = new_nodes;
			_materials = new_mats;
			_node_capacity = new_total;
			return _node_count;
		}
		uint32_t Scene::ensure_capacity(uint32_t at_least) {
			if (at_least >= _node_capacity) {
				set_capacity(((at_least / 16) + 1) * 16 );
			}
			return _node_count;
		}
		uint32_t Scene::push_node(Node node, MaterialId material) {
			ensure_capacity(_node_count + 1);
			_nodes[_node_count] = node;
			_materials[_node_count] = material;
			_node_count += 1;
			return _node_count;
		}
		void Scene::erase_node(uint32_t index) {
			_node_count -= 1;
			_nodes[index] = _nodes[_node_count];
			_materials[index] = _materials[index];
		}

		MaterialId Scene::material(uint32_t index) const {
			return _materials[index];
		}
		Node Scene::node(uint32_t index) const {
			return _nodes[index];
		}


		void Scene::accelerate_and_rebuild(Accelerator* pacc) {


			assert(pacc != _accelerator);
			if (pacc != 0) {
				delete _accelerator;
				pacc->rebuild(_adapter);
				_accelerator = pacc;
			} else if (_accelerator != 0) {
				_accelerator->rebuild(_adapter);
			} else {
				//old is 0, new is 0. Probably a mistake. Better tell the human.
				assert(0);
			}
		}
		ElementAdapter& Scene::get_adapter() {
			return _adapter;
		}
	}
}
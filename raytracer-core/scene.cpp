#include "scene.h"
#include <cstring> //memcpy
namespace rt {
	namespace core {

		glm::vec3 Node::sample_as_light(float u1, float u2, float u3) {
			glm::vec3 local_pos = shape->sample(u1, u2, u3);
			//transform to world
			glm::mat4 inv = glm::inverse(transform);
			glm::vec4 world = inv * glm::vec4(local_pos, 1);
			return glm::vec3(world);
		}

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

			//if the object is emitting add it in the lights
			Material& color = _material_bucket[material];
			if (color.emitted.x > 0.f || color.emitted.y > 0.f || color.emitted.z > 0.f) {
				_light_sources.push_back(_node_count - 1);
			}
			return _node_count;
		}
		//void Scene::erase_node(uint32_t index) {
		//	_node_count -= 1;
		//	_nodes[index] = _nodes[_node_count];
		//	_materials[index] = _materials[index];
		//}

		MaterialId Scene::material(uint32_t index) const {
			return _materials[index];
		}
		Node Scene::node(uint32_t index) const {
			return _nodes[index];
		}
		const Material& Scene::material_by_id(int id) const {
			return _material_bucket[id];
		}
		void Scene::set_material_bucket(std::vector<Material>& mat_bucket) {
			_material_bucket = mat_bucket;
		}
		MaterialId Scene::push_material(rt::core::Material& mat) {
			_material_bucket.push_back(mat);
			return _material_bucket.size() - 1;
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

		Node& Scene::sample_light(float u) const {
			int idx = round((_light_sources.size() - 1) * u);
			return _nodes[_light_sources[idx]];
		}
		const std::vector<int>& Scene::get_lights() const {
			return _light_sources;
		}

		int Scene::SceneAccAdapter::count() {
			return _scene.node_count();
		}
		AABB Scene::SceneAccAdapter::get_bounding_box(int index) const {
			AABB bbox = _scene.node(index).shape->get_bounding_box();
			glm::mat4 transform = _scene.node(index).transform;
			bbox._min = glm::vec3(transform * glm::vec4(bbox._min, 1.0f));
			bbox._max = glm::vec3(transform * glm::vec4(bbox._max, 1.0f));
			glm::vec4 corners[8];
			corners[0] = glm::vec4(bbox._min, 1.0f);
			corners[1] = glm::vec4(bbox._max, 1.0f);

			corners[2] = glm::vec4(bbox._max.x, bbox._min.y, bbox._min.z, 1.0f);
			corners[3] = glm::vec4(bbox._max.x, bbox._max.y, bbox._min.z, 1.0f);
			corners[4] = glm::vec4(bbox._min.x, bbox._max.y, bbox._min.z, 1.0f);

			corners[5] = glm::vec4(bbox._min.x, bbox._max.y, bbox._max.z, 1.0f);
			corners[6] = glm::vec4(bbox._min.x, bbox._min.y, bbox._max.z, 1.0f);
			corners[7] = glm::vec4(bbox._max.x, bbox._min.y, bbox._max.z, 1.0f);

			for (int i = 0; i < 8; ++i) {
				corners[i] = transform * corners[i];
			}

			bbox = AABB(glm::vec3(corners[0]));
			for (int i = 1; i < 8; ++i) {
				bbox = bbox.union_point(glm::vec3(corners[i]));
			}

			return bbox;
		}
		bool Scene::SceneAccAdapter::intersect(int index, Ray ray, Intersection* result) const {
			Node node = _scene.node(index);
			//convert input from world to local space
			//*** todo. Once transforms are changed this will stop working
			ray.origin = glm::vec3(node.transform * glm::vec4(ray.origin, 1));
			ray.direction = glm::vec3(node.transform * glm::vec4(ray.direction, 0));

			result->d = INFINITY;
			if (node.shape->intersect(ray, result)) {
				//convert output from local to world space
				glm::mat4 inv = glm::inverse(node.transform);
				result->normal = glm::normalize(glm::vec3(inv * glm::vec4(result->normal, 0)));
				result->position = glm::vec3(inv * glm::vec4(result->position, 1));
				result->material = _scene.material(index);
				return true;
			}
			return false;
		}

	}
}
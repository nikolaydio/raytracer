#pragma once
#include <vector>
#include <glm\glm.hpp>
#include "primitive.h"
#include "types.h"
#include "accelerator.h"

namespace rt {
	namespace core {

	


#pragma pack(push, 128)
		class Node {
		public:
			glm::mat4 transform;
			Shape* shape;
		};
#pragma pack(pop)

		class Scene {
		public:
			Scene() : _accelerator(0), _adapter(*this) {}
			~Scene() {}

			bool intersect(Ray ray, Intersection* result) const;


			//storage stuff
			uint32_t node_count() const;
			uint32_t node_capacity() const;
			uint32_t set_capacity(uint32_t new_total);
			uint32_t ensure_capacity(uint32_t at_least);

			uint32_t push_node(Node node, MaterialId material);
			void erase_node(uint32_t index);

			MaterialId material(uint32_t index) const;
			Node node(uint32_t index) const;

			void accelerate_and_rebuild(Accelerator* pacc);
			ElementAdapter& get_adapter();
		private:
			Node* _nodes;
			MaterialId* _materials;

			uint32_t _node_count;
			uint32_t _node_capacity;

			Accelerator* _accelerator;
			class SceneAccAdapter : public ElementAdapter {
			public:
				SceneAccAdapter(Scene& scene) : _scene(scene) {}
				virtual int count() {
					return _scene.node_count();
				}
				virtual AABB get_bounding_box(int index) const {
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
				virtual bool intersect(int index, Ray ray, Intersection* result) const {
					Node node = _scene.node(index);

					//convert input from world to local space
					//*** todo. Once transforms are changed this will stop working

					if (node.shape->intersect(ray, result)) {
						//convert output from local to world space
						result->material = _scene.material(index);
						return true;
					}
					return false;
				}
			private:
				Scene& _scene;
			};
			SceneAccAdapter _adapter;
		};
	}
}
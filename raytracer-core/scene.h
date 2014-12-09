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
			~Scene() {
				delete _accelerator;
			}

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
				virtual int count();
				virtual AABB get_bounding_box(int index) const;
				virtual bool intersect(int index, Ray ray, Intersection* result) const;
			private:
				Scene& _scene;
			};
			SceneAccAdapter _adapter;
		};
	}
}
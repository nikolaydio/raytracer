#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "types.h"
#include "accelerator.h"
#include "shape.h"
#include "material.h"

namespace rt {
	namespace core {




		class Node {
		public:
			glm::mat4 transform;
			Shape* shape;
		};

		class Scene {
		public:
			Scene() : _accelerator(0), _adapter(*this) {
				_node_count = 0;
				_node_capacity = 0;
				_nodes = 0;
				_materials = 0;
			}
			~Scene() {
				delete _accelerator;
				set_capacity(0);
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

			Material material_by_id(int id) const;
			void set_material_bucket(std::vector<Material>& mat_bucket);

			void accelerate_and_rebuild(Accelerator* pacc);
			ElementAdapter& get_adapter();
		private:
			Node* _nodes;
			MaterialId* _materials;

			uint32_t _node_count;
			uint32_t _node_capacity;

			Accelerator* _accelerator;
			std::vector<Material> _material_bucket;
			class SceneAccAdapter : public ElementAdapter {
			public:
				SceneAccAdapter(Scene& scene) : _scene(scene) {}
				virtual int count();
				virtual AABB get_bounding_box(int index) const;
				virtual bool intersect(int index, Ray ray, Intersection* result) const;
			private:
				Scene& _scene;
				SceneAccAdapter& operator=(const SceneAccAdapter&) {
					return *this;
				}
			};
			SceneAccAdapter _adapter;
		};
	}
}
#include "scene.h"
#include "config.h"

namespace rt{
	namespace core {
		class KdTreeAccelerator : public Accelerator {
		public:
			class KdTreeNode {
			public:
				enum NODE_TYPE {
					NT_LEAF,
					NT_INTERIOR
				} node_type;
				AABB self_aabb;
				//if leaf
				std::vector<int> elements;
				//inf interior
				float split;
				int axis;
				KdTreeNode* left;
				KdTreeNode* right;
			};
			KdTreeAccelerator(ElementAdapter& adapter) : Accelerator(adapter) {

			}
			void release(KdTreeNode* node) {
				if (node->node_type == KdTreeNode::NT_INTERIOR) {
					release(node->left);
					release(node->right);
				}
				delete node;
			}
			~KdTreeAccelerator() {
				release(root);
			}
			KdTreeNode* root;

			virtual bool intersect(Ray ray, Intersection* result) const {
				return hit(root, ray, result);
			}
			bool hit(KdTreeNode* node, Ray ray, Intersection* result) const {
				if (!node->self_aabb.intersect(ray)) {
					return false;
				}
				if (node->node_type == KdTreeNode::NT_INTERIOR) {
					Intersection left, right;
					float range_left = glm::abs(node->left->self_aabb.intersect_t(ray));
					float range_right = glm::abs(node->right->self_aabb.intersect_t(ray));
					if (range_left < range_right) {
						bool left_h = hit(node->left, ray, result);
						if (left_h) {
							return true;
						}
						return hit(node->right, ray, result);
					} else {
						bool right_h = hit(node->right, ray, result);
						if (right_h) {
							return true;
						}
						return hit(node->left, ray, result);
					}
					return false;
				} else {
					Intersection check;
					result->d = INFINITY;
					Intersection before = *result;
					for (int i = 0; i < node->elements.size(); ++i) {
						if (_adapter.intersect(node->elements[i], ray, &check)) {
							if (check.d < result->d) {
								*result = check;
							}
						}
					}
					if (before.d > result->d) {
						return true;
					}
				}
				return false;
			}
			virtual void rebuild(ElementAdapter& adapter) {
				_adapter = adapter;
				int size = adapter.count();
				AABB node_bounds;
				node_bounds._min = glm::vec3(INFINITY, INFINITY, INFINITY);
				node_bounds._max = glm::vec3(-INFINITY, -INFINITY, -INFINITY);

				int* elements = new int[size];
				for (int i = 0; i < size; ++i) {
					elements[i] = i;
					node_bounds = AABB(node_bounds, adapter.get_bounding_box(i));
				}
				root = build_tree(node_bounds, elements, size, 0);
				delete[] elements;
			}

			KdTreeNode* build_tree(AABB nodeBounds, int* elements, int nPrims, int depth) {
				KdTreeNode* node = new KdTreeNode;
				node->self_aabb = nodeBounds;
				if (depth >= KDTREE_MAX_DEPTH || nPrims <= KDTREE_MIN_ELEMENTS) {
					node->node_type = KdTreeNode::NT_LEAF;
					node->elements.resize(nPrims);
					for (int i = 0; i < nPrims; ++i) {
						node->elements[i] = elements[i];
					}
					return node;
				}

				//choose axis to split
				glm::vec3 d = nodeBounds._max - nodeBounds._min;
				int axis;
				if (d.x > d.y && d.x > d.z) axis = 0;
				else axis = (d.y > d.z) ? 1 : 2;
				
				glm::vec3 mid = (nodeBounds._min + nodeBounds._max) / 2.0f;
				AABB left = nodeBounds, right = nodeBounds;
				switch (axis) {
				case 0:
					left._max.x = mid.x;
					right._min.x = mid.x;
					break;
				case 1:
					left._max.y = mid.y;
					right._min.y = mid.y;
					break;
				case 2:
					left._max.z = mid.z;
					right._min.z = mid.z;
				};
				std::vector<int> left_list;
				std::vector<int> right_list;
				for (int i = 0; i < nPrims; ++i) {
					AABB obj = _adapter.get_bounding_box(elements[i]);
					if (left.intersect(obj)) {
						left_list.push_back(elements[i]);
					}
					if (right.intersect(obj)) {
						right_list.push_back(elements[i]);
					}
				}
				
				node->node_type = KdTreeNode::NT_INTERIOR;
				node->axis = axis;
				node->left = build_tree(left, left_list.data(), left_list.size(), depth + 1);
				node->right = build_tree(right, right_list.data(), right_list.size(), depth + 1);
				return node;
			}
		};
		Accelerator* create_kdtree(ElementAdapter& adapter) {
			return new KdTreeAccelerator(adapter);
		}
		Accelerator* create_default_accelerator(ElementAdapter& adapter) {
			return new DefaultAccelerator(adapter);
		}
	}
}

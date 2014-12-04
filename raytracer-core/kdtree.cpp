#include "scene.h"

namespace rt{
	namespace core {
		template<class T>
		class KdTreeAccelerator : public Accelerator {
		public:
			bool intersect(Ray ray, Intersection* result) const {
				return true;
			}
			void build_from(Primitive** prims, int size) {
				build_tree();
			}

			void build_tree() {
				
			}
		};
	}
}
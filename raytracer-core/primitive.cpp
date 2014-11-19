#include "primitive.h"
#include <vector>

namespace rt {
	namespace core {

		class BruteForcePrimitive : public Primitive {
			std::vector<Primitive*> _primitives;
		public:
			void add_primitive(Primitive* prim) {
				_primitives.push_back(prim);
			}
			virtual bool intersect(Ray ray, Intersection* result) const {

			}
		};

	}
}
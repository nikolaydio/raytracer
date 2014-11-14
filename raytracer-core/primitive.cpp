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
		};

		class BruteForcePrimitiveBuilder : public AggregatePrimitiveBuilder {
			BruteForcePrimitive* _primitive;
		public:
			virtual Primitive* end_primitive() {
				Primitive* result = _primitive;
				_primitive = 0;
				return result;
			}
			virtual void add_primitive(Primitive* primitive) {
				if (!_primitive)
					_primitive = new BruteForcePrimitive;
				_primitive->add_primitive(primitive);
			}
		};

		AggregatePrimitiveBuilder* create_builder_bruteforce() {
			return new BruteForcePrimitiveBuilder;
		}
	}
}
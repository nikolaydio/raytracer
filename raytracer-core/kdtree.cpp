#include "scene.h"

namespace rt{
	namespace core {
		class KdTreeAccelerator : public Accelerator {
		public:
			KdTreeAccelerator(ElementAdapter& adapter) : Accelerator(adapter) {}

			virtual bool intersect(Ray ray, Intersection* result) const {

				return true;
			}
			virtual void rebuild(ElementAdapter& adapter) {
				_adapter = adapter;
			}
		};
	}
}
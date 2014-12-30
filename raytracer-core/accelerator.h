#pragma once
#include "types.h"

namespace rt {
	namespace core {
		//class meant to abstract the input and output of accelerator classes
		class ElementAdapter {
		public:
			virtual int count() = 0;
			virtual AABB get_bounding_box(int index) const = 0;
			virtual bool intersect(int index, Ray ray, Intersection* result) const = 0;
		};
		class Accelerator {
		public:
			Accelerator(ElementAdapter& adapter) : _adapter(adapter) {}
			virtual ~Accelerator() {}

			virtual bool intersect(Ray ray, Intersection* result) const = 0;
			virtual void rebuild(ElementAdapter& adapter) = 0;
		protected:
			ElementAdapter& _adapter;
		private:
			Accelerator& operator=(const Accelerator&) {
				return *this;
			}
		};

		class DefaultAccelerator : public Accelerator {
		public:
			DefaultAccelerator(ElementAdapter& adapter) : Accelerator(adapter) {}

			virtual bool intersect(Ray ray, Intersection* result) const {
				Intersection check;
				check.d = 0;

				result->d = INFINITY;
				for (int i = 0; i < _adapter.count(); ++i) {
					if (_adapter.intersect(i, ray, &check)) {
						if (check.d < result->d) {
							*result = check;
						}
					}
				}
				if (result->d == INFINITY) {
					return false;
				}
				return true;
			}
			virtual void rebuild(ElementAdapter& adapter) {
				_adapter = adapter;
			}
		};

		Accelerator* create_kdtree(ElementAdapter& adapter);
		Accelerator* create_default_accelerator(ElementAdapter& adapter);
	}
}
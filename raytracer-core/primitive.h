#pragma once

#include "types.h"

namespace rt {
	namespace core {

		class Primitive{
		public:

			Primitive() {}
			~Primitive() {}

			virtual bool intersect(Ray ray, Intersection* result) const = 0;
			virtual AABB get_bounding_box() const = 0;
		};



		class GeoPrimitive : public Primitive {
			Shape* _shape;
			MaterialId _material;
		public:
			GeoPrimitive(Shape* shape, MaterialId mat) : _shape(shape), _material(mat) {}
			virtual bool intersect(Ray ray, Intersection* result) const {
				if (_shape->intersect(ray, result)) {
					result->material = _material;
					return true;
				}
				return false;
			}
			AABB get_bounding_box() const {
				return _shape->get_bounding_box();
			}
		};
	}
}
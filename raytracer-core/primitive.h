#pragma once

#include "types.h"

namespace rt {
	namespace core {
		//TODO:
		//Primitives should work with AABB so they can properly build internal trees.
		//Initial implementation was gonna use 4x4transforms but this seems like a bad idea.
		//Spheres need jsut an origin and radius to work efficiently.
		class Primitive{
		public:

			Primitive() {}
			~Primitive() {}

			virtual bool intersect(Ray ray, Intersection* result) const = 0;
		};
		class AggregatePrimitiveBuilder {
		public:
			//the returned primitive should be deleted when done.
			virtual Primitive* end_primitive() = 0;
			virtual void add_primitive(Primitive* primitive) = 0;
		};

		//delete this after
		AggregatePrimitiveBuilder* create_builder_bruteforce();




		class GeoPrimitive : public Primitive {
			Shape* _shape;
			Material* _material;
		public:
			GeoPrimitive(Shape* shape, Material* mat) : _shape(shape), _material(mat) {}
			virtual bool intersect(Ray ray, Intersection* result) const {
				if (_shape->intersect(ray, result)) {
					result->material = _material;
					return true;
				}
				return false;
			}
		};
	}
}
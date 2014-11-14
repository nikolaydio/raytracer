#pragma once
#include <vector>
#include <glm\glm.hpp>
#include "primitive.h"
#include "types.h"

namespace rt {
	namespace core {




		class Scene {
		public:
			Scene() {}
			~Scene() {}

			bool intersect(Ray ray, Intersection* result) const;
			void set_primitive(Primitive* prim);
		private:
			Primitive* aggregate;
		};
	}
}
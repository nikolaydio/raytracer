#pragma once
#include <vector>


namespace rt {
	namespace core {
		//Keeps the list of objects and performs the ray tracing
		//The idea is to allow it to do some of spatial paritioning.
		class RayResult{};
		class Scene {
		public:
			Scene() {}
			~Scene() {}

			RayResult pick();
		};
	}
}
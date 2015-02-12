#pragma once
#include <random>

namespace rt {
	namespace core {
		class RNG {
			std::mt19937 _gen;
			std::uniform_real_distribution<float> _dis; 
		public:
			RNG(int seed) {
				_gen.seed(seed);
				_dis = std::uniform_real_distribution<float>(0, 1);
			}
			float gen() {
				return _dis(_gen);
			}
		};
	}
}
#include "resource_manager.h"

namespace rt {
	namespace core {
		ResourceManager::~ResourceManager() {
			for (auto i = _resources.begin(), end = _resources.end(); i != end; ++i) {
				delete i->second.ptr;
			}
		}
		void* ResourceManager::resource(uint64_t path, uint32_t type, int& size) {
			ResourceKey key = { path, type };
			auto iter = _resources.find(key);
			if (iter == _resources.end()) {
				return 0;
			}
			size = iter->second.size;
			return iter->second.ptr;
		}

		//temp method
		void ResourceManager::add_resource(ResourceKey key, ResourceData data) {
			_resources.insert({ key, data });
		}
	}
}
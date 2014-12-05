#pragma once
#include <cstdint>
#include <unordered_map>
#include <functional>
#include <vector>
#include "types.h"

namespace rt {
	namespace core {
		//I may be overengineering the resource system.
		//Better leave it for later stages of development when it is truly necessary.
		//Now - just some interfaces for runtime definition of materials

		class ResourceDir {
		public:
			virtual bool exist(uint64_t path, uint16_t type);
			virtual void* load(uint64_t path, uint16_t type, int& size);
		};

		struct ResourceKey {
			uint64_t path;
			uint32_t type;
			bool operator==(const ResourceKey& key) const {
				return (path == key.path) && (type == key.type);
			}
		};

		struct ResourceData {
			void* ptr;
			int size;
		};

		//HashCollision detection should be top priotity for the later stages of dev.
		class ResourceManager {
		public:
			void* resource(uint64_t path, uint32_t type, int& size);
			~ResourceManager();

			//temp method
			void add_resource(ResourceKey key, ResourceData data);

			MaterialId add_material(Material mat);
			Material material(MaterialId mat_id);
		private:
			//std::unordered_map<ResourceKey, ResourceData> _resources;
			std::vector<Material> _materials;
		};
	}
}
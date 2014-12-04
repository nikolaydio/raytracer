#pragma once
#include <cstdint>
#include <unordered_map>
#include <functional>

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

	}
}
namespace std{
	template <>
	struct std::hash < rt::core::ResourceKey >
	{
		std::size_t operator()(const rt::core::ResourceKey& k) const
		{
			return ((k.path << 8) & 0xFFFFFF00) | (k.type & 0xFF);
		}
	};
}

namespace rt {
	namespace core {
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
		private:
			std::unordered_map<ResourceKey, ResourceData> _resources;
		};
	}
}
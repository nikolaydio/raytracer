#pragma once
#include "file_loader.h"
#include <unordered_map>
#include <string>
#include <shape.h>

namespace rt {
	namespace sdl {
		enum class ResourceType {
			MATERIAL,
			MESH,
			UNDEFINED
		};
		class Resource {
		public:
			Resource() {
				data = 0;
				size = 0;
				release_func = 0;
				type = ResourceType::UNDEFINED;
			}
			void* data;
			int size;

			void (*release_func)(Resource*);
			ResourceType type;
		};
		class ResourceCache {
			std::unordered_map<std::string, Resource> _resource_map;
			std::vector<Resource> _unnamed_resouces;
		public:
			~ResourceCache();

			void add_resource(const char* path, Resource resource);
			Resource* get_resource(const char* path);

			void release_all();
		};

		class ResourceManager {
			ResourceCache _cache;
			FileLoader& _file_loader;

			void* parse_mesh(const char* data, int* size);
		public:
			ResourceManager(FileLoader& fl) : _file_loader(fl) {}
			~ResourceManager() { cleanup(); }
			void* get_resource(const char* path, ResourceType type, int* size);
			void add_unnamed_resource(Resource res);
			void add_unnamed_shape(rt::core::Shape* shape);

			void cleanup();
		};
	}
}
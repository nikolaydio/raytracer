#include "resource_manager.h"
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/mesh.h>
#include <shape.h>


namespace rt {
	namespace sdl {
		ResourceCache::~ResourceCache() {
			release_all();
		}
		void ResourceCache::add_resource(const char* path, Resource resource) {
			if (!path) {
				_unnamed_resouces.push_back(resource);
			}else{
				_resource_map[path] = resource;
			}
		}
		Resource* ResourceCache::get_resource(const char* path) {
			auto iter = _resource_map.find(path);
			if (iter != _resource_map.end()) {
				return &iter->second;
			}
			return 0;
		}

		void ResourceCache::release_all() {
			for (auto& pair : _resource_map) {
				if (pair.second.release_func) {
					pair.second.release_func(&pair.second);
				}
			}
			_resource_map.clear();
			for (auto& res : _unnamed_resouces) {
				if (res.release_func) {
					res.release_func(&res);
				}
			}
			_unnamed_resouces.clear();
		}

		void release_shape_callback(Resource* res) {
			rt::core::Shape* shape = (rt::core::Shape*)res->data;
			delete shape;
		}

		void* ResourceManager::get_resource(const char* path, ResourceType type, int* size) {
			//If the resource is in memory, return it
			rt::sdl::Resource* res = _cache.get_resource(path);
			if (res) {
				*size = res->size;
				return res->data;
			}

			//otherwise, try to load it and parse it
			char* buffer = _file_loader.load_raw_file(path, size);
			if (!buffer) {
				return 0;
			}
			Resource resource;


			if (type == ResourceType::MESH) {
				void* mesh_object = parse_mesh((const char*)buffer, size);
				resource.data = mesh_object;
				resource.size = *size;
				resource.release_func = release_shape_callback;
				resource.type = ResourceType::MESH;
			}
			delete[] buffer;

			//If the loading and parsing are successful,
			//Add the new resource to the cache
			if (resource.data) {
				_cache.add_resource(path, resource);
			}

			*size = resource.size;
			return resource.data;
		}
		void ResourceManager::add_unnamed_resource(Resource res) {
			_cache.add_resource(0, res);
		}
		void ResourceManager::add_unnamed_shape(rt::core::Shape* shape) {
			Resource res;
			res.data = shape;
			res.size = sizeof(rt::core::Mesh);
			res.release_func = release_shape_callback;
			res.type = ResourceType::MESH;
			add_unnamed_resource(res);
		}

		void ResourceManager::cleanup() {
			_cache.release_all();
		}




		//-----------Functions for handling different type of data-----------//
		void* ResourceManager::parse_mesh(const char* data, int* size) {
			Assimp::Importer importer;


			const aiScene* scene = importer.ReadFileFromMemory(data, *size,
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_SortByPType);
			// If the import failed, report it
			if (!scene)
			{
				printf("Failed to load mesh\n");
				return 0;
			}
			rt::core::Mesh* rtmesh = new rt::core::Mesh;
			for (int i = 0; i < scene->mNumMeshes; ++i) {
				const struct aiMesh* mesh = scene->mMeshes[i];;
				for (unsigned int t = 0; t < mesh->mNumFaces; ++t)
				{
					const struct aiFace * face = &mesh->mFaces[t];

					for (int i = 2; i >= 0; --i) {
						glm::vec3 v(mesh->mVertices[face->mIndices[i]].x,
							mesh->mVertices[face->mIndices[i]].z,
							mesh->mVertices[face->mIndices[i]].y);


						rtmesh->push_vert(v);
					}


				}
			}
			rtmesh->set_accelerator(rt::core::create_kdtree(rtmesh->get_adapter()));
			*size = sizeof(rt::core::Mesh);
			return rtmesh;
		}
	}
}
#include "resource_manager.h"
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/mesh.h>
#include <assimp/texture.h>

#include <shape.h>
#include <surface.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rt {
	namespace sdl {
		ResourceCache::~ResourceCache() {
			release_all();
		}
		void ResourceCache::add_resource(const char* path, Resource resource) {
			if (!path) {
				_unnamed_resouces.push_back(resource);
			}
			else{
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

		void release_texture_callback(Resource* res) {
			rt::core::Surface2d* surf = (rt::core::Surface2d*)res->data;
			delete surf;
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
			else if (type == ResourceType::TEXTURE) {
				void* image_object = parse_image((const char*)buffer, size);
				resource.data = image_object;
				resource.size = *size;
				resource.release_func = release_texture_callback;
				resource.type = ResourceType::TEXTURE;
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
		void* ResourceManager::parse_image(const char* data, int* size) {
			int width, height, comp;
			stbi_uc* stbi = stbi_load_from_memory((const stbi_uc*)data, *size, &width, &height, &comp, STBI_rgb_alpha);
			if (!stbi) {
				return 0;
			}
			int32_t* image = (int32_t*)stbi;
			rt::core::Surface2d* surf = new rt::core::Surface2d(width, height);
			for (int u = 0; u < width; ++u) {
				for (int v = 0; v < height; ++v) {
					int32_t color = image[u + v * width];
					core::Color new_color(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color & 0xFF000000) >> 24);
					surf->pixel(u, v) = new_color;
				}
			}
			stbi_image_free(stbi);
			return surf;
		}
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
			for (int mesh_id = 0; mesh_id < scene->mNumMeshes; ++mesh_id) {
				const struct aiMesh* mesh = scene->mMeshes[mesh_id];;
				for (unsigned int t = 0; t < mesh->mNumFaces; ++t)
				{
					const struct aiFace * face = &mesh->mFaces[t];

					for (int i = 2; i >= 0; --i) {
						glm::vec3 vert(mesh->mVertices[face->mIndices[i]].x,
							mesh->mVertices[face->mIndices[i]].z,
							mesh->mVertices[face->mIndices[i]].y);


						if (mesh->HasTextureCoords(0)) {
							glm::vec2 uv(mesh->mTextureCoords[0][face->mIndices[i]].x,
								mesh->mTextureCoords[0][face->mIndices[i]].y);
							rtmesh->push_vert(vert, uv);
						} else{
							rtmesh->push_vert(vert);
						}
					}
					

				}
			}
			rtmesh->set_accelerator(rt::core::create_kdtree(rtmesh->get_adapter()));
			*size = sizeof(rt::core::Mesh);
			return rtmesh;
		}
	}
}
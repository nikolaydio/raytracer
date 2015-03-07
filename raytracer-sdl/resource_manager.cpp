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

#include <types.h>
#include <material.h>

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
						int index = face->mIndices[i];
						glm::vec3 vert(mesh->mVertices[index].x,
							mesh->mVertices[index].y,
							mesh->mVertices[index].z);


						if (mesh->HasTextureCoords(0)) {
							glm::vec2 uv(mesh->mTextureCoords[0][index].x,
								mesh->mTextureCoords[0][index].y);

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

		bool construct_mesh(const struct aiMesh* ai_mesh, rt::core::Mesh** mesh) {
			rt::core::Mesh* rtmesh = new rt::core::Mesh;
			for (unsigned int t = 0; t < ai_mesh->mNumFaces; ++t)
			{
				const struct aiFace * face = &ai_mesh->mFaces[t];

				for (int i = 2; i >= 0; --i) {
					int index = face->mIndices[i];
					glm::vec3 vert(ai_mesh->mVertices[index].x,
						ai_mesh->mVertices[index].y,
						ai_mesh->mVertices[index].z);


					if (ai_mesh->HasTextureCoords(0)) {
						glm::vec2 uv(ai_mesh->mTextureCoords[0][index].x,
							ai_mesh->mTextureCoords[0][index].y);

						rtmesh->push_vert(vert, uv);
					} else{
						rtmesh->push_vert(vert);
					}
				}
			}
			rtmesh->set_accelerator(rt::core::create_kdtree(rtmesh->get_adapter()));
			*mesh = rtmesh;
			return true;
		}
		bool get_texture(rt::sdl::ResourceManager& manager, std::shared_ptr<rt::core::ColorFilter>* target, aiTextureType type, const struct aiMaterial* ai_mat) {
			aiString name;
			//diffuse
			aiReturn r = ai_mat->GetTexture(type, 0, &name);
			if(AI_SUCCESS != r) {
				printf("Failed to grab a diffuse texture for one of the materials.\n");
				(*target).reset(new core::SpectrumFilter(glm::vec3(0.0, 0.0, 0.0)));
				return true;
			}

			std::string fn = std::string("/") + name.C_Str();
			int temp;
			rt::core::Surface2d* texture_surface = (rt::core::Surface2d*)manager.get_resource(fn.c_str(), ResourceType::TEXTURE, &temp);
			if (!texture_surface) {
				printf("Failed load texture %s.\n", fn.c_str());
				return false;
			}
			(*target).reset(new core::DirectTexFilter(*texture_surface));

		}
		bool construct_material(rt::sdl::ResourceManager& manager, const struct aiMaterial* ai_mat, rt::core::Material* mat) {
			get_texture(manager, &mat->diffuse, aiTextureType_DIFFUSE, ai_mat);
			get_texture(manager, &mat->specular, aiTextureType_SPECULAR, ai_mat);
			get_texture(manager, &mat->glossy, aiTextureType_SHININESS, ai_mat);
			

			return true;
		}
		bool construct_from_ai_node(rt::core::Scene& scene, rt::sdl::ResourceManager& manager, std::vector<rt::core::Shape*>& shapes, std::vector<rt::core::MaterialId>& mats, aiNode* ai_node, glm::mat4 transform) {
			//process current node
			glm::mat4 current_mat;
			for(int i = 0; i < 16; ++i) {
				current_mat[i/4][i%4] = ai_node->mTransformation[i/4][i%4];
			}
			std::string name = ai_node->mName.C_Str();

			transform = transform * current_mat;
			for(int i = 0; i < ai_node->mNumMeshes; ++i) {
				rt::core::Node node;
				node.transform = transform;
				node.shape = shapes[ai_node->mMeshes[i]];
				scene.push_node(node, mats[ai_node->mMeshes[i]]);
			}
			//process child notes
			for(int i = 0; i < ai_node->mNumChildren; ++i) {
				construct_from_ai_node(scene, manager, shapes, mats, ai_node->mChildren[i], transform);
			}
			return true;
		}
		bool construct_nodes_from_file(rt::core::Scene& scene, rt::sdl::ResourceManager& manager, const char* fn) {
			Assimp::Importer importer;


			const aiScene* ai_scene = importer.ReadFile(fn,
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_SortByPType);

			if (!ai_scene)
			{
				printf("Failed to load scene desc file\n");
				return 0;
			}
			//import meshes
			std::vector<rt::core::Shape*> shapes;
			std::vector<rt::core::MaterialId> materials;
			for (int mesh_id = 0; mesh_id < ai_scene->mNumMeshes; ++mesh_id) {
				const struct aiMesh* ai_mesh = ai_scene->mMeshes[mesh_id];
				const struct aiMaterial* ai_mat = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

				rt::core::Mesh* mesh;
				rt::core::Material mat;
				if(!construct_mesh(ai_mesh, &mesh)) {
					return false;
				}
				if(!construct_material(manager, ai_mat, &mat)) {
					return false;
				}

				rt::core::MaterialId mat_id = scene.push_material(mat);
				materials.push_back(mat_id);
				shapes.push_back(mesh);
				//rt::core::Node node;
				//node.shape = mesh;
				//scene.push_node(node, mat_id);
			}

			//for each ai node/mesh pair construct a new node.
			construct_from_ai_node(scene, manager, shapes, materials, ai_scene->mRootNode, glm::mat4());
			return true;
		}
	}
}
#include "scene_loader.h"
#include "sjson_compiler.h"

#include <string>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <iostream>
#include "resource_manager.h"
#include "path.h"
#include "config.h"


rt::core::Shape* make_mesh(const char* filename);
namespace rt {
	namespace sdl {
		config_file load_config_file(const char* fn, FileLoader& loader) {
			std::string str;
			bool success = loader.load_string_file(fn, &str);
			if (!success) {
				std::cout << "Failed to open file " << fn << std::endl;
				return 0;
			}

			json_file* file = sjson_compile_source(str.c_str());
			if (!file) {
				std::cout << "Failed to compile " << fn << std::endl;
			}
			return file;
		}
		void free_config_file(config_file f) {
			sjson_free_file((json_file*)f);
		}

		bool load_scene_and_accelerate(config_file f, rt::core::Scene& scene, ResourceManager& manager) {
			std::vector<rt::core::Material> material_list;

			json_file* file = (json_file*)f;
			std::cout << "Loading " << sjson_table_string(file, SJSON_ROOT_TABLE_ID, "name") << "\n";
			int node_list = sjson_table_int(file, SJSON_ROOT_TABLE_ID, "nodelist");
			int node_count = sjson_list_entry_count(file, node_list);
			std::cout << "Objects count " << node_count << "\n";

			for (int i = 0; i < node_count; ++i) {
				int node_id = sjson_list_int(file, node_list, i);
				rt::core::Node node;
				int transform_id = sjson_table_int(file, node_id, "transform");
				for (int i = 0; i < 16; ++i) {
					node.transform[i / 4][i % 4] = sjson_list_float(file, transform_id, i);
				}

				int shape_id = sjson_table_int(file, node_id, "shape");
				if (strcmp(sjson_table_string(file, shape_id, "type"), "mesh") == 0) {
					if (sjson_table_lookup(file, shape_id, "verts") == AT_LIST) {
						rt::core::Mesh* mesh = new rt::core::Mesh;
						int vert_list = sjson_table_int(file, shape_id, "verts");
						int vert_count = sjson_list_entry_count(file, vert_list) / 3;
						for (int i = 0; i < vert_count; ++i) {
							glm::vec3 vert(sjson_list_float(file, vert_list, i * 3),
								sjson_list_float(file, vert_list, i * 3 + 1),
								sjson_list_float(file, vert_list, i * 3 + 2));
							mesh->push_vert(vert);
						}
						mesh->set_accelerator(new rt::core::DefaultAccelerator(mesh->get_adapter()));
						node.shape = mesh;
					}
					else{
						const char* filename = sjson_table_string(file, shape_id, "source");
						int temp_size;
						void* mesh = manager.get_resource(filename, ResourceType::MESH, &temp_size);
						if (!mesh) {
							sjson_free_file(file);
							return false;
						}
						node.shape = (rt::core::Shape*)mesh;
					}
				}
				else if (strcmp(sjson_table_string(file, shape_id, "type"), "sphere") == 0){
					float radius = sjson_table_float(file, shape_id, "radius");
					node.shape = new rt::core::Sphere(glm::vec3(0, 0, 0), radius);
				}
				else{
					sjson_free_file(file);
					return false;
				}

				rt::core::Material mat;
				int mat_id = sjson_table_int(file, node_id, "material");
				int emit_id = sjson_table_int(file, mat_id, "emit");
				int reflect_id = sjson_table_int(file, mat_id, "reflect");
				for (int i = 0; i < 3; ++i) {
					mat.emitted[i] = sjson_list_float(file, emit_id, i);
					mat.reflected[i] = sjson_list_float(file, reflect_id, i);
				}
				if (sjson_table_lookup(file, mat_id, "specular") == AT_INT) {
					mat.specular = true;
				}
				material_list.push_back(mat);
				scene.push_node(node, material_list.size()-1);
			}

			const char* accelerator_name = sjson_table_string(file, SJSON_ROOT_TABLE_ID, "accelerator");
			rt::core::Accelerator* accelerator = 0;
			if (strcmp(accelerator_name, "kdtree") == 0) {
				accelerator = rt::core::create_kdtree(scene.get_adapter());
			}
			else if (strcmp(accelerator_name, "std") == 0) {
				accelerator = rt::core::create_default_accelerator(scene.get_adapter());
			}
			else {
				accelerator = rt::core::create_kdtree(scene.get_adapter());
			}
			scene.accelerate_and_rebuild(accelerator);

			scene.set_material_bucket(material_list);
			return true;
		}
		bool load_images(config_file f, std::vector<rt::core::Renderer>& renderers, rt::core::Scene& scene, std::vector<rt::core::Surface2d*>& surfaces, core::MemoryArena& arena) {
			json_file* file = (json_file*)f;
			int images_id = sjson_table_int(file, SJSON_ROOT_TABLE_ID, "images");
			for (int i = 0; i < sjson_list_entry_count(file, images_id); ++i) {
				int current_image_id = sjson_list_int(file, images_id, i);
				
				if (strcmp(sjson_table_string(file, current_image_id, "method"), "path") != 0) {
					std::cout << "Unknown method param. The only valid value is 'path'\n";
					return false;
				}
				int samples = sjson_table_int(file, current_image_id, "samples");

				//parse the camera definitions
				int cam_id = sjson_table_int(file, current_image_id, "camera");
				if (strcmp(sjson_table_string(file, cam_id, "type"), "pinhole") != 0) {
					std::cout << "Unknown camera type. The only valid value is 'pinhole'\n";
					return false;
				}
				int cam_eye_id = sjson_table_int(file, cam_id, "eye");
				int cam_lookat_id = sjson_table_int(file, cam_id, "lookat");
				glm::vec3 cam_eye, lookat;
				for (int i = 0; i < 3; ++i) {
					cam_eye[i] = sjson_list_float(file, cam_eye_id, i);
					lookat[i] = sjson_list_float(file, cam_lookat_id, i);
				}
				float fov = sjson_table_float(file, cam_id, "fov");

				bool take_normals = false;
				if (sjson_table_lookup(file, current_image_id, "take_normals") == AT_INT) {
					take_normals = sjson_table_int(file, current_image_id, "take_normals") == 1;
				}

				rt::core::Sampler* sampler = ARENA_NEW(arena, rt::core::Sampler, samples);
				rt::core::Integrator* path = ARENA_NEW(arena, rt::core::Path);
				rt::core::Camera* camera = ARENA_NEW(arena, rt::core::Camera, cam_eye, lookat, fov, ((float)WND_SIZE_X / (float)WND_SIZE_Y));
				
				
				surfaces.push_back(new rt::core::Surface2d(WND_SIZE_X, WND_SIZE_Y));
				rt::core::Film* radiance_film = ARENA_NEW(arena, rt::core::Film, surfaces.back());

				rt::core::Film* normals_film = 0;
				if (take_normals) {
					surfaces.push_back(new rt::core::Surface2d(WND_SIZE_X, WND_SIZE_Y));
					normals_film = ARENA_NEW(arena, rt::core::Film, surfaces.back());;
				}

				rt::core::Renderer renderer(*sampler, *camera, scene, *path, radiance_film, normals_film);
				renderers.push_back(renderer);
			}
		}

	}
}
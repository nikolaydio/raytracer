#include "scene_loader.h"
#include "sjson_compiler.h"

#include <string>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <iostream>
#include "resource_manager.h"

rt::core::Shape* make_mesh(const char* filename);
namespace rt {
	namespace sdl {

		bool SceneLoader::load_from(const char* filename, rt::core::ResourceManager& manager, core::Scene& scene) {
			std::ifstream t(filename);
			if (!t.is_open()) {
				return false;
			}
			std::string str;

			t.seekg(0, std::ios::end);
			str.reserve(t.tellg());
			t.seekg(0, std::ios::beg);

			str.assign((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());

			t.close();

			json_file* file = sjson_compile_source(str.c_str());
			if (!file) {
				return false;
			}
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
						mesh->set_accelerator(rt::core::create_kdtree(mesh->get_adapter()));
						node.shape = mesh;
					}
					else{
						node.shape = make_mesh(sjson_table_string(file, shape_id, "source"));
					}
				}else{
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
				scene.push_node(node, manager.add_material(mat));
			}
			sjson_free_file(file);
			return true;
		}
	}
}
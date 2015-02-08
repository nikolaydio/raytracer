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
		void ConfigFileDeleter::operator()(ConfigFile* p) {
			sjson_free_file((json_file*)p);
		}

		std::unique_ptr<ConfigFile, ConfigFileDeleter> load_config_file(const char* fn, FileLoader& loader) {
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
			return std::unique_ptr<ConfigFile, ConfigFileDeleter>((ConfigFile*)file);
		}
		void free_config_file(ConfigFile* f) {
			sjson_free_file((json_file*)f);
		}





		//------------------Helper functions and macros------------------//

		void dump_consumer_stack(std::vector<std::string>& consumer_stack) {
			std::cout << "Scene desc parser stack:\n";
			for (auto& str : consumer_stack) {
				std::cout << "\t" << str << std::endl;
			}
		}


#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

		class StackFrame__ {
			std::vector<std::string>& _stack;
		public:
			StackFrame__(std::vector<std::string>& stack, const char* str1, int line) : _stack(stack) {
				_stack.push_back(str1);
				_stack.back() += " : ";
				_stack.back() += std::to_string(line);
			}
			~StackFrame__() {
				_stack.pop_back();
			}
		};

#define ABORT_LOADING(stack, msg) { dump_consumer_stack(stack); std::cout << msg << std::endl; return false; }

#define ENSURE_TYPE(stack, obj, checked_type) { if(obj.type != checked_type) \
	{ ABORT_LOADING(stack, "Expected type: " STRINGIZE_DETAIL(checked_type)) } }
#define ENSURE_EITHER_TYPE(stack, obj, checked_type, checked_type2) \
	if(obj.type != checked_type && obj.type != checked_type2) \
	{ ABORT_LOADING(stack, "Expected type: " STRINGIZE_DETAIL(checked_type)) }

#define ENSURE_EQUAL(stack, first, second) if(first != second) { dump_consumer_stack(stack); std::cout << "Expected val: " << STRINGIZE_DETAIL(second); return false; }
#define STACK_FRAME(stack, name, obj) StackFrame__ ____stack__frame____(stack, name, sjson_object_line(obj));


		//--------------------Scene NodeList Parser Recursive funcions----------------------//
		bool consume_transform(std::vector<std::string>& stack, JS_Object obj, glm::mat4* transform) {
			STACK_FRAME(stack, "Transform", obj);
			int count = sjson_object_count(obj);
			ENSURE_EQUAL(stack, count, 16);
			for (int i = 0; i < 16; ++i) {
				JS_Object val = sjson_object_child(obj, i);
				ENSURE_EITHER_TYPE(stack, val, AT_FLOAT, AT_INT);
				(*transform)[i / 4][i % 4] = sjson_object_float(val);
			}
			return true;
		}
		bool consume_shape(std::vector<std::string>& stack, JS_Object obj, ResourceManager& manager, rt::core::Shape** shape) {
			STACK_FRAME(stack, "Shape", obj);

			JS_Object mesh_type = sjson_object_child(obj, "type");
			ENSURE_TYPE(stack, mesh_type, AT_STRING);
			const char* mesh_type_str = sjson_object_string(mesh_type);
			
			if (strcmp(mesh_type_str, "mesh") == 0) {

				//Mesh is loaded either directly(inline) or from file(source)
				JS_Object source = sjson_object_child(obj, "source");
				if (source.type == AT_STRING) {
					const char* source_str = sjson_object_string(source);
					int temp_size;
					void* mesh = manager.get_resource(source_str, ResourceType::MESH, &temp_size);
					if (!mesh) {
						ABORT_LOADING(stack, "Failed to load mesh from source " << source_str);
					}
					*shape = (rt::core::Shape*)mesh;
				} else {
					//verify that we are loading from verts
					JS_Object verts = sjson_object_child(obj, "verts");
					ENSURE_TYPE(stack, verts, AT_LIST);
					int vert_component_count = sjson_object_count(verts);
					if (vert_component_count % 3 != 0) {
						ABORT_LOADING(stack, "Vertex component count must be multiple of 3.");
					}
					int vert_count = vert_component_count / 3;
					rt::core::Mesh* mesh = new rt::core::Mesh;
					for (int i = 0; i < vert_count; ++i) {
						glm::vec3 vert;

						for (int u = 0; u < 3; ++u) {
							JS_Object vert_cmp = sjson_object_child(verts, i * 3 + u);
							ENSURE_EITHER_TYPE(stack, vert_cmp, AT_FLOAT, AT_INT);
							vert[u] = sjson_object_float(vert_cmp);
						}

						mesh->push_vert(vert);
					}
					//the default accelerator for inline meshes is default accelerator
					//as it has minimal overhead, and inline meshes are usually a few faces
					mesh->set_accelerator(new rt::core::DefaultAccelerator(mesh->get_adapter()));
					*shape = mesh;
					//this will transfer the ownership to the mesh manager so it can be freed when the time comes
					manager.add_unnamed_shape(mesh);
				}
			}else if (strcmp(mesh_type_str, "sphere") == 0) {
				JS_Object sphere_radius = sjson_object_child(obj, "radius");
				ENSURE_EITHER_TYPE(stack, sphere_radius, AT_FLOAT, AT_INT);
				float radius = sjson_object_float(sphere_radius);
				*shape = new rt::core::Sphere(glm::vec3(0, 0, 0), radius);
				manager.add_unnamed_shape(*shape);
			}else{
				ABORT_LOADING(stack, "Unknown mesh type");
			}

			return true;
		}
		bool consume_vector(std::vector<std::string>& stack, JS_Object obj, glm::vec3* res) {
			STACK_FRAME(stack, "Vector", obj);
			int components = sjson_object_count(obj);
			if (components != 3) {
				ABORT_LOADING(stack, "Vector list must contain exactly 3 numbers.");
			}
			for (int i = 0; i < 3; ++i) {
				JS_Object component = sjson_object_child(obj, i);
				ENSURE_EITHER_TYPE(stack, component, AT_FLOAT, AT_INT);
				(*res)[i] = sjson_object_float(component);
			}
			return true;
		}
		bool consume_filter(std::vector<std::string>& stack, JS_Object obj, ResourceManager& manager, std::shared_ptr<rt::core::ColorFilter>* filter) {
			STACK_FRAME(stack, "ColorSource", obj);
			if (obj.type == AT_STRING) {
				const char* source = sjson_object_string(obj);
				int temp;
				rt::core::Surface2d* texture_surface = (rt::core::Surface2d*)manager.get_resource(source, ResourceType::TEXTURE, &temp);
				if (!texture_surface) {
					ABORT_LOADING(stack, "Failed to load texture from source " << source);
				}
				(*filter).reset(new core::BilinearFilter(*texture_surface));
			}else{
				glm::vec3 out;
				if (!consume_vector(stack, obj, &out)) {
					return false;
				}
				(*filter).reset(new core::SpectrumFilter(out));
			}
			return true;
		}
		bool consume_material(std::vector<std::string>& stack, JS_Object obj, rt::core::Scene& scene, ResourceManager& manager, rt::core::MaterialId* id) {
			STACK_FRAME(stack, "Material", obj);
			rt::core::Material mat;
			JS_Object emit_vector = sjson_object_child(obj, "emit");
			JS_Object diffuse_obj = sjson_object_child(obj, "diffuse");
			JS_Object specular_obj = sjson_object_child(obj, "specular");
			JS_Object glossy_obj = sjson_object_child(obj, "glossy");

			if (!consume_vector(stack, emit_vector, &mat.emitted)) {
				return false;
			}
			
			if (diffuse_obj.type != AT_NONE) {
				if (!consume_filter(stack, diffuse_obj, manager, &mat.diffuse)) {
					return false;
				}
			}
			if (specular_obj.type != AT_NONE) {
				if (!consume_filter(stack, specular_obj, manager, &mat.specular)) {
					return false;
				}
			}
			if (glossy_obj.type != AT_NONE) {
				if (!consume_filter(stack, glossy_obj, manager, &mat.glossy)) {
					return false;
				}
			}

			*id = scene.push_material(mat);
			return true;
		}
		bool consume_single_node(std::vector<std::string>& stack, JS_Object obj, rt::core::Scene& scene, ResourceManager& manager) {
			STACK_FRAME(stack, "Single Node", obj);
			JS_Object node_obj(obj);
			rt::core::Node node;

			JS_Object transform = sjson_object_child(node_obj, "transform");
			ENSURE_TYPE(stack, transform, AT_LIST);
			if (!consume_transform(stack, transform, &node.transform)) {
				return false;
			}

			JS_Object shape = sjson_object_child(node_obj, "shape");
			ENSURE_TYPE(stack, shape, AT_TABLE);
			if (!consume_shape(stack, shape, manager, &node.shape)) {
				return false;
			}

			JS_Object material = sjson_object_child(node_obj, "material");
			ENSURE_TYPE(stack, material, AT_TABLE);
			rt::core::MaterialId material_id;
			if (!consume_material(stack, material, scene, manager, &material_id)) {
				return false;
			}
			scene.push_node(node, material_id);
			return true;
		}
		bool consume_nodelist(std::vector<std::string>& stack, JS_Object obj, rt::core::Scene& scene, ResourceManager& manager) {
			STACK_FRAME(stack, "Node List", obj);
			int node_count = sjson_object_count(obj);
			for (int i = 0; i < node_count; ++i) {
				JS_Object node = sjson_object_child(obj, i);
				ENSURE_TYPE(stack, node, AT_TABLE);
				if (!consume_single_node(stack, node, scene, manager)) {
					return false;
				}
			}
			return true;
		}
		
		bool load_scene_and_accelerate(const ConfigFile& f, rt::core::Scene& scene, ResourceManager& manager) {
			std::vector<std::string> consumer_stack;

			json_file* file = (json_file*)&f;
			JS_Object root = sjson_object_root(file);
			STACK_FRAME(consumer_stack, "Scene root", root);


			JS_Object scene_name = sjson_object_child(root, "name");
			ENSURE_TYPE(consumer_stack, scene_name, AT_STRING);
			std::cout << "Loading " << sjson_object_string(scene_name) << "\n";


			JS_Object nodelist = sjson_object_child(root, "nodelist");
			ENSURE_TYPE(consumer_stack, nodelist, AT_LIST);
			std::cout << "Object count: " << sjson_object_count(nodelist) << std::endl;
			if (!consume_nodelist(consumer_stack, nodelist, scene, manager)) {
				return false;
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
			return true;
		}
		bool load_images(const ConfigFile& f, std::vector<rt::core::Renderer>& renderers, rt::core::Scene& scene, std::vector<rt::core::Film*>& films, core::MemoryArena& arena) {
			json_file* file = (json_file*)&f;
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

				rt::core::Sampler* sampler = ARENA_NEWV(arena, rt::core::Sampler, samples);
				rt::core::Integrator* path = ARENA_NEW(arena, rt::core::Path);
				rt::core::Camera* camera = ARENA_NEWV(arena, rt::core::Camera, cam_eye, lookat, fov, ((float)WND_SIZE_X / (float)WND_SIZE_Y));
				
				
				rt::core::Surface2d* radiance_surf = new rt::core::Surface2d(WND_SIZE_X, WND_SIZE_Y);
				rt::core::Film* radiance_film = new rt::core::Film(radiance_surf);
				films.push_back(radiance_film);

				rt::core::Film* normals_film = 0;
				if (take_normals) {
					rt::core::Surface2d* normals_surf = new rt::core::Surface2d(WND_SIZE_X, WND_SIZE_Y);
					normals_film = new rt::core::Film(normals_surf);
					films.push_back(normals_film);
				}

				rt::core::Renderer renderer(*sampler, *camera, scene, *path, radiance_film, normals_film);
				renderers.push_back(renderer);
			}
		}

	}
}
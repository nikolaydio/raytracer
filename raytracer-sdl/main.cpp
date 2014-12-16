#include "renderer.h"
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/mesh.h>
#include "shape.h"

#define WND_SIZE_X 640
#define WND_SIZE_Y 480

#define REQUIRE(cond) do { if(!(cond)) { printf("Cond failed %s: %s\n", __FILE__, __LINE__); exit(0); }  }while(0)




rt::core::Shape* make_mesh(const char* filename) {
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType);
	// If the import failed, report it
	if (!scene)
	{
		return 0;
	}
	rt::core::Mesh* rtmesh = new rt::core::Mesh;;
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
	return rtmesh;
}
void build_scene(rt::core::ResourceManager& manager, rt::core::Scene* scene) {
	rt::core::MaterialId left_sph_mat =
		manager.add_material({ glm::vec3(1, 1, 1), glm::vec3(0.0, 0.0, 0.0) });

	rt::core::MaterialId right_sph_mat =
		manager.add_material({ glm::vec3(0.7, 0.49, 0.43), glm::vec3(0.0, 0.0, 0.0) });

	rt::core::MaterialId right_tri_mat =
		manager.add_material({ glm::vec3(0.72, 0.76, 0.73), glm::vec3(0.0, 0.0, 0.0) });

	rt::core::MaterialId white =
		manager.add_material({ glm::vec3(1, 1, 1), glm::vec3(1, 1, 1) });

	//scene->push_node({ glm::mat4(), new Sphere(glm::vec3(0, 0, 0), 1) }, left_sph_mat);
	glm::mat4 mesh_trans;
	//mesh_trans = glm::scale(mesh_trans, glm::vec3(5, 5, 5));
	mesh_trans = glm::rotate(mesh_trans, 55.0f, glm::vec3(1.f, 0.f, 0.f));
	mesh_trans = glm::translate(mesh_trans, glm::vec3(-0.7, 1.4, 0));
	scene->push_node({ mesh_trans, make_mesh("chasha.dae") }, left_sph_mat);
	glm::mat4 sph_trans;
	sph_trans = glm::translate(sph_trans, glm::vec3(0.8, 0.0, 0.4));
	scene->push_node({ sph_trans, new rt::core::Sphere(glm::vec3(0, 0, 0), 0.6) }, right_sph_mat);



	float height = -5.0f;
	rt::core::Shape* shape = new rt::core::Triangle(glm::vec3(-3000, height, -300), glm::vec3(0, height, 300), glm::vec3(3000, height, -300));
	scene->push_node({ glm::mat4(), shape }, right_tri_mat);

	float pos = 15;
	shape = new rt::core::Triangle(glm::vec3(-300, -300, pos), glm::vec3(0, 200, pos), glm::vec3(300, -300, pos));
	scene->push_node({ glm::mat4(), shape }, right_tri_mat);

	pos = 15;
	shape = new rt::core::Triangle(glm::vec3(100000, 10000, pos), glm::vec3(0, -10000, -100.f), glm::vec3(-100000, 10000, pos));
	scene->push_node({ glm::mat4(), shape }, white);
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

void test() {

}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	test();
#endif
	rt::core::ResourceManager manager;

	rt::core::Surface2d film_surface(WND_SIZE_X, WND_SIZE_Y);

	rt::core::Film film(&film_surface);

	rt::core::Camera cam(glm::vec3(-2, -0.1, -5), glm::vec3(0, -0.3, 0), 60, (float)WND_SIZE_X / (float)WND_SIZE_Y);

	rt::core::Scene scene;
	build_scene(manager, &scene);

	scene.accelerate_and_rebuild(new rt::core::DefaultAccelerator(scene.get_adapter()));

	rt::core::Sampler sampler(1);
	rt::core::Integrator integrator;

	rt::core::Renderer renderer(sampler, cam, scene, integrator, manager);
	renderer.film() = &film;

	auto begin = std::chrono::high_resolution_clock::now();
	renderer.run_multithreaded();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
	double seconds = (double)duration / 1000.0 / 1000.0 / 1000.0;
	std::cout << "Render time: " << seconds << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to init SDL");
		exit(0);
	}
	SDL_Window* window = SDL_CreateWindow("Raytracer", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, WND_SIZE_X, WND_SIZE_Y, 0);
	REQUIRE(window);

	//make sure we do not have to do some kind of resizing
	//later may be supported but for *now* - no
	int w = 0, h = 0;
	SDL_GetWindowSize(window, &w, &h);
	REQUIRE(w == WND_SIZE_X);
	REQUIRE(h == WND_SIZE_Y);


	SDL_Surface* surface = SDL_GetWindowSurface(window);
	REQUIRE(surface);


	bool running = true;
	while (running) {

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT) {
				running = false;
			}
		}

		SDL_LockSurface(surface);
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				putpixel(surface, x, y, *(Uint32*)&film_surface.pixel(x, y));
			}
		}
		SDL_UnlockSurface(surface);

		SDL_UpdateWindowSurface(window);
		SDL_Delay(120);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
#include "renderer.h"
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/mesh.h>


#define WND_SIZE_X 640
#define WND_SIZE_Y 480

#define REQUIRE(cond) do { if(!(cond)) { printf("Cond failed %s: %s\n", __FILE__, __LINE__); exit(0); }  }while(0)

bool IntersectSphere(glm::vec3 sphere_pos, float radius, glm::vec3 rpos, glm::vec3 rdir, float* dist_out) {
	glm::vec3 L = sphere_pos - rpos;
	float tca = glm::dot(L, rdir);
	if (tca < 0) return false;
	float d2 = glm::dot(L, L) - tca * tca;
	if (d2 > radius) return false;
	float thc = glm::sqrt(radius - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;


	if (t0 > t1) {
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	if (t1 < 0)
		return false;

	if (t0 < 0){
		*dist_out = t1;
		return true;
	}
	else{
		*dist_out = t0;
		return true;
	}
}

class Sphere : public rt::core::Shape {
	glm::vec3 _pos;
	float _radius;
public:
	Sphere(glm::vec3 pos, float radius)
		: _pos(pos), _radius(radius) {}
	virtual bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
		if (IntersectSphere(_pos, _radius, ray.origin, ray.direction, &result->d)) {
			result->position = (ray.origin + ray.direction * result->d);
			result->normal = glm::normalize(result->position - _pos);
			return true;
		}
		return false;
	}
	rt::core::AABB get_bounding_box() const {
		return rt::core::AABB(_pos - _radius, _pos + _radius);
	}
};

class Triangle : public rt::core::Shape {
	glm::vec3 p0, p1, p2;
public:
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) : p0(a), p1(b), p2(c) {}

	bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
		glm::vec3 e1 = p1 - p0;
		glm::vec3 e2 = p2 - p0;
		glm::vec3 s1 = glm::cross(ray.direction, e2);
		float divisor = glm::dot(s1, e1);

		if (glm::abs(divisor) < 0.00000001) {
			return false;
		}
		float invDivisor = 1.f / divisor;
		//first barycentric coordinate
		glm::vec3 d = ray.origin - p0;
		float b1 = glm::dot(d, s1) * invDivisor;
		if (b1 < 0. || b1 > 1.) {
			return false;
		}
		//second
		glm::vec3 s2 = glm::cross(d, e1);
		float b2 = glm::dot(ray.direction, s2) * invDivisor;
		if (b2 <0. || b1 + b2 > 1.) {
			return false;
		}
		float t = glm::dot(e2, s2) * invDivisor;
		
		//this is a threashhold for not colliding with itself
		if (t < 0.0001) {
			return false;
		}
		result->d = t;
		result->position = ray.origin + ray.direction * t;
		result->normal = glm::normalize(glm::cross(e1, e2));
		return true;
	}
	rt::core::AABB get_bounding_box() const {
		return rt::core::AABB(p0, p1, p2);
	}
};

class Mesh : public rt::core::Shape {
	std::vector<glm::vec3> points;
	class MeshAdapter : public rt::core::ElementAdapter {
		std::vector<glm::vec3>& _mesh;

	public:
		MeshAdapter(std::vector<glm::vec3>& mesh) : _mesh(mesh) {}
		virtual int count() {
			return _mesh.size() / 3;
		}
		virtual rt::core::AABB get_bounding_box(int index) const {
			rt::core::AABB aabb(
				_mesh[index * 3], _mesh[index * 3 + 1], _mesh[index * 3 + 2]);

			return aabb;
		}
		virtual bool intersect(int index, rt::core::Ray ray, rt::core::Intersection* result) const {
			Triangle tri(_mesh[index * 3], _mesh[index * 3 + 1], _mesh[index * 3 + 2]);
			return tri.intersect(ray, result);
		}
	};
	MeshAdapter _adapter;
	rt::core::Accelerator* _accelerator;
	rt::core::AABB bbox;
public:
	Mesh() : _accelerator(0), _adapter(points) { }
	rt::core::ElementAdapter& get_adapter() {
		return _adapter;
	}
	void push_face(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
		points.push_back(a);
		points.push_back(b);
		points.push_back(c);
	}
	void push_vert(glm::vec3 a) {
		points.push_back(a);
	}
	void set_accelerator(rt::core::Accelerator* acc) {
		_accelerator = acc;
		acc->rebuild(_adapter);
		bbox = get_bounding_box();
	}
	bool intersect(rt::core::Ray ray, rt::core::Intersection* result) const {
		if (!bbox.intersect(ray)) {
			return false;
		}
		return _accelerator->intersect(ray, result);
	}
	rt::core::AABB get_bounding_box() const {
		rt::core::AABB aabb(points[0]);
		for (int i = 1; i < points.size(); ++i) {
			aabb = aabb.union_point(points[i]);
		}
		return aabb;
	}
};

#define MURMUR_STRING(str, seed) (rt::core::MurmurHash2(str, strlen(str), seed))
rt::core::MaterialId make_material(const char* path, rt::core::ResourceManager& man, rt::core::Material mat) {
	rt::core::Material* pmat = new rt::core::Material(mat);
	rt::core::MaterialId hash = rt::core::MurmurHash2(path, strlen(path), 0);
	uint32_t type_hash = MURMUR_STRING(".material", 0);

	//man.add_resource({ hash, type_hash }, { pmat , sizeof(rt::core::Material) });
	man.add_material(mat);
	return hash;
}

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
	Mesh* rtmesh = new Mesh;;
	const struct aiMesh* mesh = scene->mMeshes[0];;
	for (unsigned int t = 0; t < mesh->mNumFaces; ++t)
	{
		const struct aiFace * face = &mesh->mFaces[t];

		for (int i = 2; i >= 0; --i) {
			glm::vec3 v(mesh->mVertices[face->mIndices[i]].x,
				mesh->mVertices[face->mIndices[i]].y,
				mesh->mVertices[face->mIndices[i]].z);


			rtmesh->push_vert(v);
		}


	}
	rtmesh->set_accelerator(rt::core::create_kdtree(rtmesh->get_adapter()));
	return rtmesh;
}
void build_scene(rt::core::ResourceManager& manager, rt::core::Scene* scene) {
	rt::core::MaterialId left_sph_mat =
		manager.add_material({ glm::vec3(1, 1, 1), glm::vec3(0.2, 0.86, 0.45) });

	rt::core::MaterialId right_sph_mat =
		manager.add_material({ glm::vec3(1, 1, 1), glm::vec3(0.9, 0.04, 0.7) });

	rt::core::MaterialId right_tri_mat =
		manager.add_material({ glm::vec3(1, 1, 1), glm::vec3(0.4, 0.4, 0.4) });

	rt::core::MaterialId white =
		manager.add_material({ glm::vec3(0, 0, 0), glm::vec3(1, 1, 1) });

	//scene->push_node({ glm::mat4(), new Sphere(glm::vec3(0, 0, 0), 1) }, left_sph_mat);
	glm::mat4 mesh_trans(1.);
	mesh_trans = glm::rotate(mesh_trans, 140.0f, glm::vec3(1.f, 0.f, 0.f));
	mesh_trans = glm::translate(mesh_trans, glm::vec3(1, -0, 0));
	scene->push_node({ mesh_trans, make_mesh("chasha.dae") }, left_sph_mat);
	scene->push_node({ glm::mat4(), new Sphere(glm::vec3(2, -0.6, 0), 1.25) }, right_sph_mat);



	float height = -5.0f;
	rt::core::Shape* shape = new Triangle(glm::vec3(-3000, height, -300), glm::vec3(0, height, 300), glm::vec3(3000, height, -300));
	scene->push_node({ glm::mat4(), shape }, right_tri_mat);

	float pos = 15;
	shape = new Triangle(glm::vec3(-300, -300, pos), glm::vec3(0, 200, pos), glm::vec3(300, -300, pos));
	scene->push_node({ glm::mat4(), shape }, right_tri_mat);

	pos = 15;
	shape = new Triangle(glm::vec3(1000, 300, pos), glm::vec3(0, -30.f, -40.f), glm::vec3(-1000, 300, pos));
	//scene->push_node({ glm::mat4(), shape }, white);
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
	{
	Triangle a(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
	rt::core::Intersection isect;
	rt::core::Ray ray;
	ray.origin = glm::vec3(0.1, 0.1, -1);
	ray.direction = glm::vec3(0, 0, 1);
	assert(a.intersect(ray, &isect));
	assert(isect.normal == glm::vec3(0, 0, -1));
	assert(isect.position == glm::vec3(0.1, 0.1, 0));

	ray.origin = glm::vec3(1, 1, -1);
	assert(!a.intersect(ray, &isect));

	ray.origin = glm::vec3(0.1, 0.1, 1);
	ray.direction = glm::vec3(0, 0, -1);
	assert(a.intersect(ray, &isect));
	assert(isect.normal == glm::vec3(0, 0, -1));
	assert(isect.position == glm::vec3(0.1, 0.1, 0));

	float fa = -INFINITY;
	float fb = 10;
	assert(!(fa > fb));
	assert((fa < fb));
	}






	glm::vec4 a(0, 0, 0, 1), b(1, 0, 0, 0), c(1, 1, 1, 0);
	glm::mat4 transform;
	transform = glm::rotate(transform, 90.0f, glm::vec3(0, 1, 0));
	glm::vec4 a1 = transform * a;
	glm::vec4 b1 = transform * b;
	glm::vec4 c1 = transform * c;

	glm::vec4 a2 = glm::inverse(transform) * a1;
	glm::vec4 b2 = glm::inverse(transform) * b1;
	glm::vec4 c2 = glm::inverse(transform) * c1;
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
		uint32_t* pixels = (uint32_t*)surface->pixels;
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
#include "accelerator.h"
#include "gtest\gtest.h"
#include <random>
#include <scene.h>

TEST(kdtree, kdtree_impl)
{

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-1, 1);

	rt::core::Mesh mesh;
	const float scale = 50.0f;
	for (int i = 0; i < 5000; ++i) {
		mesh.push_face(glm::vec3(dis(gen), dis(gen), dis(gen)) * scale,
			glm::vec3(dis(gen), dis(gen), dis(gen)) * scale,
			glm::vec3(dis(gen), dis(gen), dis(gen)) * scale);
	}
	rt::core::Accelerator* kdtree = rt::core::create_kdtree(mesh.get_adapter());
	rt::core::Accelerator* std_a = new rt::core::DefaultAccelerator(mesh.get_adapter());
	kdtree->rebuild(mesh.get_adapter());
	std_a->rebuild(mesh.get_adapter());

	rt::core::Ray ray;
	rt::core::Intersection temp1, temp2;
	for (int i = 0; i < 1000; ++i) {
		ray.origin = glm::vec3(dis(gen), dis(gen), dis(gen)) * scale;
		ray.direction = glm::normalize(glm::vec3(dis(gen), dis(gen), dis(gen)));

		bool r1 = kdtree->intersect(ray, &temp1);
		bool r2 = std_a->intersect(ray, &temp2);
		EXPECT_EQ(r1, r2);
		if (r1 == r2 && r1 == true) {
			EXPECT_EQ(temp1.d, temp2.d);
			EXPECT_EQ(temp1.normal, temp2.normal);
		}
	}
}
TEST(accelerators, accelerator_impl) {
	rt::core::Mesh mesh;
	mesh.push_face(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 1, 0));
	mesh.push_face(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 1));
	
	rt::core::Accelerator* std_a = new rt::core::DefaultAccelerator(mesh.get_adapter());
	std_a->rebuild(mesh.get_adapter());

	rt::core::Ray ray;
	rt::core::Intersection temp1, temp2;

	ray.origin = glm::vec3(0.5, 0.75, -1);
	ray.direction = glm::vec3(0, 0, 1);
	bool res = std_a->intersect(ray, &temp1);
	EXPECT_EQ(true, res);
	EXPECT_EQ(1.0f, temp1.d);
	EXPECT_EQ(glm::vec3(0.5, 0.75, 0), temp1.position);

	ray.origin = glm::vec3(1.4, 0.75, -1);
	ray.direction = glm::vec3(0, 0, 1);
	res = std_a->intersect(ray, &temp1);
	EXPECT_EQ(false, res);

	
}
TEST(accelerators, kdtree_impl) {
	rt::core::Mesh mesh;
	mesh.push_face(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 1, 0));
	mesh.push_face(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 1));

	rt::core::Accelerator* std_a = rt::core::create_kdtree(mesh.get_adapter());
	std_a->rebuild(mesh.get_adapter());

	rt::core::Ray ray;
	rt::core::Intersection temp1, temp2;

	ray.origin = glm::vec3(0.5, 0.75, -1);
	ray.direction = glm::vec3(0, 0, 1);
	bool res = std_a->intersect(ray, &temp1);
	EXPECT_EQ(true, res);
	EXPECT_EQ(1.0f, temp1.d);
	EXPECT_EQ(glm::vec3(0.5, 0.75, 0), temp1.position);

	ray.origin = glm::vec3(1.4, 0.75, -1);
	ray.direction = glm::vec3(0, 0, 1);
	res = std_a->intersect(ray, &temp1);
	EXPECT_EQ(false, res);


}
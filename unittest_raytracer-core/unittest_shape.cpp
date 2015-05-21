
#include "gtest/gtest.h"
#include "shape.h"


TEST(shape, shape_triangle)
{
	rt::core::Triangle a(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0),
		glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0));
	rt::core::Intersection isect;
	rt::core::Ray ray;
	ray.origin = glm::vec3(0.1, 0.1, -1);
	ray.direction = glm::vec3(0, 0, 1);
	EXPECT_EQ(true, a.intersect(ray, &isect));
	EXPECT_EQ(glm::vec3(0, 0, -1), isect.normal);
	EXPECT_EQ(glm::vec3(0.1, 0.1, 0), isect.position);

	ray.origin = glm::vec3(1, 1, -1);
	EXPECT_EQ(false, a.intersect(ray, &isect));

	ray.origin = glm::vec3(0.1, 0.1, 1);
	ray.direction = glm::vec3(0, 0, -1);
	EXPECT_EQ(true, a.intersect(ray, &isect));
	EXPECT_EQ(glm::vec3(0, 0, -1), isect.normal);
	EXPECT_EQ(glm::vec3(0.1, 0.1, 0), isect.position);

}
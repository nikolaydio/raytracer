
#include "gtest/gtest.h"
#include "camera.h"


TEST(camera, camera_rays)
{
	//not a complete unit test. just tests the basics

	glm::vec3 origin = glm::vec3(0, 0, 0);
	glm::vec3 dir = glm::vec3(1, 0, 0);
	rt::core::Camera cam(origin, dir, 60.0f, 640. / 480.);
	rt::core::Ray ray;
	rt::core::Sample sample1, sample2; sample1.position = glm::vec2(0.5, 0.5);
	cam.find_ray(sample1, &ray);
	EXPECT_EQ(origin, ray.origin);
	EXPECT_EQ(dir, ray.direction);
	EXPECT_EQ(1.0f, glm::length(ray.direction));

	sample1.position = glm::vec2(0.25, 0.25);
	sample2.position = glm::vec2(0.75, 0.75);
	rt::core::Ray ray1, ray2;
	cam.find_ray(sample1, &ray1);
	cam.find_ray(sample2, &ray2);
	EXPECT_EQ(1.0f, glm::length(ray1.direction));
	EXPECT_EQ(1.0f, glm::length(ray2.direction));

	glm::vec3 expected = ray.origin + ray.direction;
	glm::vec3 exp2 = expected - (ray2.origin + ray2.direction);
	glm::vec3 exp1 = expected - (ray1.origin + ray1.direction);
	EXPECT_EQ(glm::length(exp1), glm::length(exp2));
}
#include "camera.hpp"

Camera::Camera(const glm::mat3& projection)
	: facing(0, 1)
	, projection(projection)
{

}

void Camera::set_projection(const glm::mat3& projection)
{
	this->projection = projection;
}

void Camera::set_facing(const glm::vec2& facing)
{
	this->facing = glm::normalize(facing);
}

void Camera::set_origin(const glm::vec2& origin)
{
	this->origin = origin;
}

void Camera::recalculate_matrices()
{
	//glm::vec2 right = glm::vec2(facing.y, -facing.x);
	//glm::mat3 rotation = glm::mat3(right.x,  right.y,  0,
	//							   facing.x, facing.y, 0,
	//							   0,        0,        1);
	//glm::mat3 translation = glm::mat3(1,		 0,			0,
	//								  0,		 1,			0,
	//								  -origin.x, -origin.y, 1);
	//view = rotation * translation;

	glm::vec2 right = glm::vec2(facing.y, -facing.x);
	view[0][0] = right.x;
	view[1][0] = right.y;
	view[0][1] = facing.x;
	view[1][1] = facing.y;
	view[2][0] = -glm::dot(right, origin);
	view[2][1] = -glm::dot(facing, origin);

	//float angle = -std::atan2(facing.y, facing.x);
	//glm::mat3 rotation = glm::mat3(std::cos(angle),	 std::sin(angle), 0,
	//							   -std::sin(angle), std::cos(angle), 0,
	//							   0,				 0,				  1);
	//glm::mat3 translation = glm::mat3(1,		 0,			0,
	//								  0,		 1,			0,
	//							      -origin.x, -origin.y, 1);
	//view = rotation * translation;
}

const glm::vec2& Camera::get_facing() const
{
	return facing;
}

glm::vec2 Camera::get_right() const
{
	return glm::vec2(facing.y, -facing.x);
}

const glm::vec2& Camera::get_origin() const
{
	return origin;
}

const glm::mat3& Camera::get_view() const
{
	return view;
}

const glm::mat3& Camera::get_projection() const
{
	return projection;
}

glm::mat3 Camera::create_projection(float scale, float viewport_width, float viewport_height)
{
	float aspect = viewport_height / viewport_width;
	return glm::mat3(scale * aspect, 0,		0,
					 0,				 scale,	0,
					 0,				 0,		1);
}

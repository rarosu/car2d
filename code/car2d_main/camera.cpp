#include "camera.hpp"

Camera::Camera()
	: facing(0, 1)
{

}

void Camera::set_scale(float scale)
{
	projection = glm::mat3(scale, 0,	 0,
						   0,	  scale, 0,
						   0,	  0,	 1);
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
	glm::vec2 right = glm::vec2(facing.y, -facing.x);
	glm::mat3 rotation = glm::mat3(right.x,  right.y,  0,
								   facing.x, facing.y, 0,
								   0,        0,        1);
	glm::mat3 translation = glm::mat3(1,		 0,			0,
									  0,		 1,			0,
									  -origin.x, -origin.y, 1);
	view = rotation * translation;
}

const glm::vec2& Camera::get_facing() const
{
	return facing;
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

#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	Camera(const glm::mat3& projection);

	void set_projection(const glm::mat3& projection);
	void set_facing(const glm::vec2& facing);
	void set_origin(const glm::vec2& origin);
	void recalculate_matrices();
	const glm::vec2& get_facing() const;
	glm::vec2 get_right() const;
	const glm::vec2& get_origin() const;
	const glm::mat3& get_view() const;
	const glm::mat3& get_projection() const;

	static glm::mat3 create_projection(float scale, float viewport_width, float viewport_height);
private:
	glm::vec2 facing;
	glm::vec2 origin;
	glm::mat3 view;
	glm::mat3 projection;
};
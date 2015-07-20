#pragma once

#include <iostream>
#include <glm/glm.hpp>

std::ostream& operator<<(std::ostream& stream, const glm::vec2& vector);
std::ostream& operator<<(std::ostream& stream, const glm::vec3& vector);

std::ostream& operator<<(std::ostream& stream, const glm::mat2& matrix);
std::ostream& operator<<(std::ostream& stream, const glm::mat3& matrix);
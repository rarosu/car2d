#include "debug.hpp"

std::ostream& operator<<(std::ostream& stream, const glm::vec2& vector)
{
	return stream << "(" << vector.x << ", " << vector.y << ")";
}

std::ostream& operator<<(std::ostream& stream, const glm::vec3& vector)
{
	return stream << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
}

std::ostream& operator<<(std::ostream& stream, const glm::mat2& matrix)
{
	const char* SPACE = " ";
	stream << matrix[0][0] << SPACE << matrix[1][0] << std::endl;
	stream << matrix[0][1] << SPACE << matrix[1][1] << std::endl;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat3& matrix)
{
	const char* SPACE = " ";
	stream << matrix[0][0] << SPACE << matrix[1][0] << SPACE << matrix[2][0] << std::endl;
	stream << matrix[0][1] << SPACE << matrix[1][1] << SPACE << matrix[2][1] << std::endl;
	stream << matrix[0][2] << SPACE << matrix[1][2] << SPACE << matrix[2][2] << std::endl;
	return stream;
}
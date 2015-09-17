#pragma once

#define NOMINMAX

#include <yaml-cpp/yaml.h>
#include <GL/gl3w.h>
#include "camera.hpp"
#include "config.hpp"

struct TerrainPerInstance
{
	glm::mat3x4 model_matrix;
	glm::mat3x4 bias_matrix;
	glm::mat3x4 inverse_projection_view;
};

class Terrain
{
public:
	Terrain(const YAML::Node& map_file, const Camera& camera);
	~Terrain();

	void render(const Camera& camera);
private:
	TerrainPerInstance uniform_instance_data;
	GLuint uniform_instance_buffer;
	GLuint quad_position_vbo;
	GLuint quad_texcoord_vbo;
	GLuint quad_vao;
	GLuint texture;
	GLuint sampler;
	GLuint terrain_vs;
	GLuint terrain_fs;
	GLuint terrain_program;
};
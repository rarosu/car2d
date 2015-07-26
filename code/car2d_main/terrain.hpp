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
};

class Terrain
{
public:
	Terrain(const YAML::Node& map_file);
	~Terrain();

	void render();
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
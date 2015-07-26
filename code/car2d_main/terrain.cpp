#include "terrain.hpp"
#include <gli/gli.hpp>
#include "shader.hpp"

Terrain::Terrain(const YAML::Node& map_file)
{
	float scale = map_file["GroundTextureScale"].as<float>();
	glm::mat3 model = glm::mat3(scale, 0,	   0,
								0,	   scale,  0,
								0,	   0,	   1);
	glm::mat3 bias = glm::mat3(2,  0,  0,
							   0,  2,  0,
						       -1, -1, 1);
	uniform_instance_data.model_matrix = glm::mat3x4(model);
	uniform_instance_data.bias_matrix = glm::mat3x4(bias);

	glGenBuffers(1, &uniform_instance_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TerrainPerInstance), &uniform_instance_data, GL_STATIC_DRAW);

	glm::vec2 positions[] = { glm::vec2(-1.0f, 1.0f), glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, -1.0f) };
	glm::vec2 texcoords[] = { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f) };

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glGenBuffers(1, &quad_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &quad_texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gli::storage teximage = gli::load_dds(DIRECTORY_TEXTURES + map_file["GroundTexture"].as<std::string>());

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, teximage.dimensions(0).x, teximage.dimensions(0).y, 0, GL_BGR, GL_UNSIGNED_BYTE, teximage.data());

	terrain_vs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_TERRAIN_VS, GL_VERTEX_SHADER);
	terrain_fs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_TERRAIN_FS, GL_FRAGMENT_SHADER);
	terrain_program = glCreateProgram();
	glAttachShader(terrain_program, terrain_vs);
	glAttachShader(terrain_program, terrain_fs);
	link_program(terrain_program);
}

Terrain::~Terrain()
{
	glDetachShader(terrain_program, terrain_vs);
	glDetachShader(terrain_program, terrain_fs);
	glDeleteShader(terrain_vs);
	glDeleteShader(terrain_fs);
	glDeleteProgram(terrain_program);

	glDeleteSamplers(1, &sampler);
	glDeleteTextures(1, &texture);
	
	glDeleteVertexArrays(1, &quad_vao);
	glDeleteBuffers(1, &quad_position_vbo);
	glDeleteBuffers(1, &quad_texcoord_vbo);

	glDeleteBuffers(1, &uniform_instance_buffer);
}

void Terrain::render()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glUseProgram(terrain_program);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	
	glActiveTexture(GL_TEXTURE0 + TEXTURE_DIFFUSE_BINDING);
	glBindSampler(TEXTURE_DIFFUSE_BINDING, sampler);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
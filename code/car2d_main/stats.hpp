#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <freetype-gl/freetype-gl.h>

class Car;

struct TextPerInstance
{
	glm::mat3x4 model_matrix;
	glm::vec4 color;
};

class Stats
{
public:
	Stats(int viewport_width, int viewport_height);
	~Stats();

	void append_update_line(const std::string& key, const char* format, ...);
	void update_text();
	void window_resized(int viewport_width, int viewport_height);
	void render();
private:
	struct Line
	{
		std::string key;
		std::string text;
	};

	static const wchar_t* Stats::FONT_CHARSET_CACHE;
	static const float MARGIN_X;
	static const float MARGIN_Y;

	std::vector<Line> lines;
	texture_atlas_t* texture_atlas;
	texture_font_t* texture_font;
	size_t vertex_count;
	GLuint position_vbo;
	GLuint texcoord_vbo;
	GLuint vao;
	GLuint sampler;
	GLuint text_vs;
	GLuint text_fs;
	GLuint text_program;
	GLuint uniform_instance_buffer;
	TextPerInstance uniform_instance_data;
};
#include "stats.hpp"
#include "config.hpp"
#include "shader.hpp"

const wchar_t* Stats::FONT_CHARSET_CACHE =
	L" !\"#$%&'()*+,-./0123456789:;<=>?"
	L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	L"`abcdefghijklmnopqrstuvwxyz{|}~";
const float Stats::MARGIN_X = -0.95f;
const float Stats::MARGIN_Y = 0.95f;

Stats::Stats(int viewport_width, int viewport_height)
	: texture_atlas(0)
	, texture_font(0)
	, vertex_count(0)
	, position_vbo(0)
	, texcoord_vbo(0)
	, vao(0)
	, sampler(0)
	, text_vs(0)
	, text_fs(0)
	, text_program(0)
	, uniform_instance_buffer(0)
{
	texture_atlas = texture_atlas_new(512, 512, 1);
	texture_font = texture_font_new_from_file(texture_atlas, 11, (DIRECTORY_FONTS + FILE_DEFAULT_FONT).c_str());
	texture_font_load_glyphs(texture_font, FONT_CHARSET_CACHE);

	// Create a sampler object for the font.
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Load the shader program.
	text_vs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_TEXT_VS, GL_VERTEX_SHADER);
	text_fs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_TEXT_FS, GL_FRAGMENT_SHADER);
	text_program = glCreateProgram();
	glAttachShader(text_program, text_vs);
	glAttachShader(text_program, text_fs);
	link_program(text_program);

	glUniformBlockBinding(text_program, glGetUniformBlockIndex(text_program, "PerInstance"), UNIFORM_INSTANCE_BINDING);

	// Setup a scale matrix to go from screen space to normalized device space.
	glm::mat3 scale(2.0f / viewport_width, 0.0f,				   0.0f,
					0.0f,				   2.0f / viewport_height, 0.0f,
					0.0f,				   0.0f,				   1.0f);

	glm::mat3 translation(1.0f,		0.0f,	  0.0f,
						  0.0f,		1.0f,	  0.0f,
						  MARGIN_X, MARGIN_Y, 1.0f);
	uniform_instance_data.model_matrix = glm::mat3x4(translation * scale);
	uniform_instance_data.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

	glGenBuffers(1, &uniform_instance_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TextPerInstance), &uniform_instance_data, GL_DYNAMIC_DRAW);
}

Stats::~Stats()
{
	texture_atlas_delete(texture_atlas);

	glDetachShader(text_program, text_vs);
	glDetachShader(text_program, text_fs);
	glDeleteShader(text_vs);
	glDeleteShader(text_fs);
	glDeleteProgram(text_program);

	glDeleteSamplers(1, &sampler);

	glDeleteBuffers(1, &position_vbo);
	glDeleteBuffers(1, &texcoord_vbo);
	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &uniform_instance_buffer);
}

void Stats::append_update_line(const std::string& key, const char* format, ...)
{
	// Find an existing line or append a new line.
	int i = 0;
	for (; i < lines.size(); ++i)
	{
		if (lines[i].key == key)
			break;
	}

	if (i == lines.size())
	{
		lines.push_back(Line());
		lines[i].key = key;
	}

	// Update the text.
	char buffer[512];
	va_list v;
	va_start(v, format);
	if (vsprintf_s(buffer, format, v) < 0)
	{
		va_end(v);
		throw std::runtime_error("Failed to append/update stats line: buffer error");
	}
	va_end(v);

	// Copy the buffer.
	lines[i].text = buffer;
}

void Stats::update_text()
{
	// Destroy any old vertex buffers.
	glDeleteBuffers(1, &position_vbo);
	glDeleteBuffers(1, &texcoord_vbo);
	glDeleteVertexArrays(1, &vao);
	vertex_count = 0;

	// Bail out if we do not have any text.
	if (lines.size() == 0)
		return;

	// Count the number of vertices.
	for (int i = 0; i < lines.size(); ++i)
	{
		vertex_count += lines[i].text.size() * 6;
	}

	// Generate the new vertices.
	glm::vec2 pen;
	std::vector<glm::vec2> positions(vertex_count);
	std::vector<glm::vec2> texcoords(vertex_count);
	int offset = 0;
	for (int i = 0; i < lines.size(); ++i)
	{
		const std::string& text = lines[i].text;
		int text_length = text.size();

		pen.x = 0;
		for (int k = 0; k < text_length; ++k)
		{
			wchar_t c = (wchar_t) text[k];
			texture_glyph_t* glyph = texture_font_get_glyph(texture_font, c);

			float kerning = 0.0f;
			if (k > 0)
			{
				kerning = texture_glyph_get_kerning(glyph, text[k - 1]);
			}

			pen.x += kerning;

			float x0 = pen.x + glyph->offset_x;
			float y0 = pen.y + glyph->offset_y;
			float x1 = x0 + glyph->width;
			float y1 = y0 - glyph->height;

			positions[offset + k * 6 + 0] = glm::vec2(x0, y0);
			positions[offset + k * 6 + 1] = glm::vec2(x0, y1);
			positions[offset + k * 6 + 2] = glm::vec2(x1, y1);
			positions[offset + k * 6 + 3] = glm::vec2(x0, y0);
			positions[offset + k * 6 + 4] = glm::vec2(x1, y1);
			positions[offset + k * 6 + 5] = glm::vec2(x1, y0);

			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;

			texcoords[offset + k * 6 + 0] = glm::vec2(s0, t0);
			texcoords[offset + k * 6 + 1] = glm::vec2(s0, t1);
			texcoords[offset + k * 6 + 2] = glm::vec2(s1, t1);
			texcoords[offset + k * 6 + 3] = glm::vec2(s0, t0);
			texcoords[offset + k * 6 + 4] = glm::vec2(s1, t1);
			texcoords[offset + k * 6 + 5] = glm::vec2(s1, t0);

			pen.x += glyph->advance_x;
		}

		pen.y -= (texture_font->ascender - texture_font->descender) + texture_font->linegap;
		offset += text_length * 6;
	}

	// Generate new vertex buffers.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertex_count, &positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertex_count, &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void Stats::window_resized(int viewport_width, int viewport_height)
{
	// Setup a scale matrix to go from screen space to normalized device space.
	glm::mat3 scale(2.0f / viewport_width, 0.0f,				   0.0f,
					0.0f,				   2.0f / viewport_height, 0.0f,
					0.0f,				   0.0f,				   1.0f);

	glm::mat3 translation(1.0f,		0.0f,	  0.0f,
						  0.0f,		1.0f,	  0.0f,
						  MARGIN_X, MARGIN_Y, 1.0f);
	uniform_instance_data.model_matrix = glm::mat3x4(translation * scale);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TextPerInstance), &uniform_instance_data);
}

void Stats::render()
{
	if (vertex_count == 0)
		return;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUseProgram(text_program);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_DIFFUSE_BINDING);
	glBindSampler(TEXTURE_DIFFUSE_BINDING, sampler);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->id);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

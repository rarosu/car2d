#include "road.hpp"
#include "shader.hpp"
#include <gli/gli.hpp>

Road::Road(const YAML::Node& map_file)
	: segments(map_file["Segments"].size())
{
	// Load the road segments.
	for (int i = 0; i < segments.size(); ++i)
	{
		const YAML::Node& segment_node = map_file["Segments"][i];

		std::string type = segment_node["Type"].as<std::string>();
		if (type == "Straight")
		{
			segments[i] = new RoadSegmentStraight(glm::vec2(segment_node["Start"][0].as<float>(), segment_node["Start"][1].as<float>()),
												  glm::vec2(segment_node["End"][0].as<float>(), segment_node["End"][1].as<float>()));
			segments[i]->construct_buffers(1.0f, segment_node["Width"].as<float>(), 0.0f, 0.1f);
		}
		else if (type == "BezierQuadratic")
		{
			segments[i] = new RoadSegmentBezierQuadratic(glm::vec2(segment_node["Start"][0].as<float>(), segment_node["Start"][1].as<float>()),
														 glm::vec2(segment_node["Control"][0].as<float>(), segment_node["Control"][1].as<float>()),
														 glm::vec2(segment_node["End"][0].as<float>(), segment_node["End"][1].as<float>()));
			segments[i]->construct_buffers(1.0f, segment_node["Width"].as<float>(), 0.0f, 0.1f);
		}
		else if (type == "Arc")
		{
			segments[i] = new RoadSegmentArc(glm::vec2(segment_node["Center"][0].as<float>(), segment_node["Center"][1].as<float>()),
											 glm::vec2(segment_node["Start"][0].as<float>(), segment_node["Start"][1].as<float>()),
											 glm::vec2(segment_node["End"][0].as<float>(), segment_node["End"][1].as<float>()));
			segments[i]->construct_buffers(1.0f, segment_node["Width"].as<float>(), 0.0f, 0.1f);
		}
	}

	// Setup the program.
	mesh_vs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_MESH2D_VS, GL_VERTEX_SHADER);
	mesh_fs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_MESH2D_FS, GL_FRAGMENT_SHADER);
	mesh_program = glCreateProgram();
	glAttachShader(mesh_program, mesh_vs);
	glAttachShader(mesh_program, mesh_fs);
	link_program(mesh_program);

	// Setup the uniform buffer.
	uniform_instance_data.model_matrix = glm::mat3x4(glm::mat3(1.0f));
	
	glGenBuffers(1, &uniform_instance_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstance), &uniform_instance_data, GL_STATIC_DRAW);

	// Load the texture and create a sampler.
	gli::storage teximage = gli::load_dds(DIRECTORY_TEXTURES + map_file["Texture"].as<std::string>());

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, teximage.dimensions(0).x, teximage.dimensions(0).y, 0, GL_BGR, GL_UNSIGNED_BYTE, teximage.data());

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Road::~Road()
{
	for (int i = 0; i < segments.size(); ++i)
	{
		delete segments[i];
	}

	glDeleteBuffers(1, &uniform_instance_buffer);
	glDetachShader(mesh_program, mesh_vs);
	glDetachShader(mesh_program, mesh_fs);
	glDeleteProgram(mesh_program);
	glDeleteShader(mesh_vs);
	glDeleteShader(mesh_fs);
	glDeleteSamplers(1, &sampler);
	glDeleteTextures(1, &texture);
}

void Road::render()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUseProgram(mesh_program);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	
	glActiveTexture(GL_TEXTURE0 + TEXTURE_DIFFUSE_BINDING);
	glBindSampler(TEXTURE_DIFFUSE_BINDING, sampler);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	for (int i = 0; i < segments.size(); ++i)
	{
		glBindVertexArray(segments[i]->road_vao);
		glDrawArrays(GL_TRIANGLES, 0, segments[i]->road_vertex_count);
	}
}

RoadSegment::RoadSegment()
	: road_position_vbo(0)
	, road_texcoord_vbo(0)
	, road_vao(0)
	, road_vertex_count(0)
{

}

RoadSegment::~RoadSegment()
{
	glDeleteVertexArrays(1, &road_vao);
	glDeleteBuffers(1, &road_position_vbo);
	glDeleteBuffers(1, &road_texcoord_vbo);
}

float RoadSegment::get_length() const
{
	return get_length(1.0f);
}

void RoadSegment::construct_buffers(float step_length, float road_width, float shoulder_width, float texcoord_scale)
{
	std::vector<glm::vec2> road_positions;
	std::vector<glm::vec2> road_texcoords;
	float texcoord_accumulator = 0.0f;
	float step_accumulator = 0.0f;
	float length = get_length();
	int step_count = static_cast<int>(glm::ceil(length / step_length));
	for (int i = 0; i < step_count; ++i)
	{
		float t1 = get_parameter_at_distance(step_accumulator);
		step_accumulator = glm::min(step_accumulator + step_length, length);
		float t2 = get_parameter_at_distance(step_accumulator);
		
		glm::vec2 p1 = get_position(t1);
		glm::vec2 p2 = get_position(t2);

		glm::vec2 n1 = get_normal(t1);
		glm::vec2 n2 = get_normal(t2);

		road_positions.push_back(glm::vec2(p2 - n2 * road_width));
		road_positions.push_back(glm::vec2(p1 - n1 * road_width));
		road_positions.push_back(glm::vec2(p1 + n1 * road_width));
		road_positions.push_back(glm::vec2(p2 - n2 * road_width));
		road_positions.push_back(glm::vec2(p1 + n1 * road_width));
		road_positions.push_back(glm::vec2(p2 + n2 * road_width));

		road_texcoords.push_back(glm::vec2(0.0f, texcoord_accumulator + texcoord_scale));
		road_texcoords.push_back(glm::vec2(0.0f, texcoord_accumulator));
		road_texcoords.push_back(glm::vec2(1.0f, texcoord_accumulator));
		road_texcoords.push_back(glm::vec2(0.0f, texcoord_accumulator + texcoord_scale));
		road_texcoords.push_back(glm::vec2(1.0f, texcoord_accumulator));
		road_texcoords.push_back(glm::vec2(1.0f, texcoord_accumulator + texcoord_scale));
		texcoord_accumulator += texcoord_scale;
		if (texcoord_accumulator >= 1.0f)
			texcoord_accumulator = 0.0f;
	}

	road_vertex_count = road_positions.size();

	glGenVertexArrays(1, &road_vao);
	glBindVertexArray(road_vao);
	
	glGenBuffers(1, &road_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, road_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * road_positions.size(), &road_positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	glGenBuffers(1, &road_texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, road_texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * road_texcoords.size(), &road_texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

RoadSegmentStraight::RoadSegmentStraight(const glm::vec2& start, const glm::vec2& end)
	: start(start)
	, end(end)
	, length(glm::length(end - start))
{

}

glm::vec2 RoadSegmentStraight::get_position(float t) const
{
	return start + get_tangent(t) * length * t;
}

glm::vec2 RoadSegmentStraight::get_normal(float t) const
{
	glm::vec2 tangent = get_tangent(t);
	return glm::vec2(tangent.y, -tangent.x);
}

glm::vec2 RoadSegmentStraight::get_tangent(float t) const
{
	return (end - start) / length;
}

float RoadSegmentStraight::get_length(float t) const
{
	return length * t;
}

float RoadSegmentStraight::get_parameter_at_distance(float distance) const
{
	return distance / length;
}


const int RoadSegmentBezierQuadratic::INTERPOLATION_SEGMENT_COUNT = 32;
const float RoadSegmentBezierQuadratic::INTERPOLATION_DELTA = 1.0f / INTERPOLATION_SEGMENT_COUNT;

RoadSegmentBezierQuadratic::RoadSegmentBezierQuadratic(const glm::vec2& start, const glm::vec2& control, const glm::vec2& end)
	: start(start)
	, control(control)
	, end(end)
{

}

glm::vec2 RoadSegmentBezierQuadratic::get_position(float t) const
{
	float it = 1.0f - t;
	return it * it * start +
		   2.0f * it * t * control +
		   t * t * end;
}

glm::vec2 RoadSegmentBezierQuadratic::get_normal(float t) const
{
	glm::vec2 tangent = get_tangent(t);
	return glm::vec2(tangent.y, -tangent.x);
}

glm::vec2 RoadSegmentBezierQuadratic::get_tangent(float t) const
{
	return glm::normalize(-2 * (1 - t) * start + 2 * (1 - 2 * t) * control + 2 * t * end);
}

float RoadSegmentBezierQuadratic::get_length(float t) const
{
	float length = 0.0f;
	int segment = int(t * INTERPOLATION_SEGMENT_COUNT);
	for (int i = 0; i < segment; ++i)
	{
		glm::vec2 segstart = get_position(i * INTERPOLATION_DELTA);
		glm::vec2 segend = get_position((i + 1) * INTERPOLATION_DELTA);
		length += glm::length(segend - segstart);
	}

	glm::vec2 segstart = get_position(segment * INTERPOLATION_DELTA);
	glm::vec2 segend = get_position((segment + 1) * INTERPOLATION_DELTA);
	length += glm::length(segend - segstart) * (t - segment * INTERPOLATION_DELTA);

	return length;
}

float RoadSegmentBezierQuadratic::get_parameter_at_distance(float distance) const
{
	float t = 0.0f;
	float length = 0.0f;
	int segment = 0;
	while (true)
	{
		glm::vec2 segstart = get_position(t);
		glm::vec2 segend = get_position(t + INTERPOLATION_DELTA);
		float seglength = glm::length(segend - segstart);
		if (length + seglength <= distance)
		{
			t += INTERPOLATION_DELTA;
			length += seglength;
			segment++;
		}
		else
		{
			break;
		}
	}

	glm::vec2 segstart = get_position(t);
	glm::vec2 segend = get_position(t + INTERPOLATION_DELTA);
	float seglength = glm::length(segend - segstart);
	t += INTERPOLATION_DELTA * (distance - length) / seglength;

	return t;
}


RoadSegmentArc::RoadSegmentArc(const glm::vec2& center, const glm::vec2& start, const glm::vec2& end)
	: center(center)
	, radius(glm::length(start - center))
	, start_angle(std::atan2((start - center).y, (start - center).x))
	, end_angle(std::atan2((end - center).y, (end - center).x))
	, sign(glm::sign(end_angle - start_angle))
{

}

glm::vec2 RoadSegmentArc::get_position(float t) const
{
	float angle = start_angle + (end_angle - start_angle) * t;
	return center + radius * glm::vec2(std::cos(angle), std::sin(angle));
}

glm::vec2 RoadSegmentArc::get_normal(float t) const
{
	glm::vec2 tangent = get_tangent(t);
	return glm::vec2(tangent.y, -tangent.x);
}

glm::vec2 RoadSegmentArc::get_tangent(float t) const
{
	float angle = start_angle + (end_angle - start_angle) * t;
	return glm::vec2(-std::sin(angle), std::cos(angle)) * sign;
}

float RoadSegmentArc::get_length(float t) const
{
	return (end_angle - start_angle) * t * radius * sign;
}

float RoadSegmentArc::get_parameter_at_distance(float distance) const
{
	return distance / (radius * (end_angle - start_angle)) * sign;
}
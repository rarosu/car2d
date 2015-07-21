#pragma once

#define NOMINMAX

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <GL/gl3w.h>
#include <vector>
#include "config.hpp"

class RoadSegment;

class Road
{
public:
	Road(const YAML::Node& map_file);

	void render();
private:
	std::vector<RoadSegment*> segments;

	PerInstance uniform_instance_data;
	GLuint mesh_vs;
	GLuint mesh_fs;
	GLuint mesh_program;
	GLuint uniform_instance_buffer;
};

class RoadSegment
{
public:
	GLuint road_position_vbo;
	GLuint road_texcoord_vbo;
	GLuint road_vao;
	GLuint road_vertex_count;

	/* Initialize empty buffers. */
	RoadSegment();

	/* Get the position at t-value in [0, 1] */
	virtual glm::vec2 get_position(float t) const = 0;

	/* Get the right normal at t-value in [0, 1] */
	virtual glm::vec2 get_normal(float t) const = 0;

	/* Get the tangent at t-value in [0, 1] */
	virtual glm::vec2 get_tangent(float t) const = 0;

	/* Get the length up to t-value in [0, 1] */
	virtual float get_length(float t) const = 0;

	/* Get the length of the entire segment. */
	float get_length() const;

	/* Get t-value at specified distance in [0, get_length()]*/
	virtual float get_parameter_at_distance(float distance) const = 0;

	/* Setup the OpenGL buffers for this segment. */
	void construct_buffers(float step_length, float road_width, float shoulder_width);
};

/* A straight line. */
class RoadSegmentStraight : public RoadSegment
{
public:
	RoadSegmentStraight(const glm::vec2& start, const glm::vec2& end);

	glm::vec2 get_position(float t) const;
	glm::vec2 get_normal(float t) const;
	glm::vec2 get_tangent(float t) const;
	float get_length(float t) const;
	float get_parameter_at_distance(float distance) const;
private:
	glm::vec2 start;
	glm::vec2 end;
	float length;
};

/* A quadratic bezier curve. */
class RoadSegmentBezierQuadratic : public RoadSegment
{
public:
	static const int INTERPOLATION_SEGMENT_COUNT;
	static const float INTERPOLATION_DELTA;

	RoadSegmentBezierQuadratic(const glm::vec2& start, const glm::vec2& control, const glm::vec2& end);

	glm::vec2 get_position(float t) const;
	glm::vec2 get_normal(float t) const;
	glm::vec2 get_tangent(float t) const;
	float get_length(float t) const;
	float get_parameter_at_distance(float distance) const;
private:
	glm::vec2 start;
	glm::vec2 control;
	glm::vec2 end;
};

/* A circle arc. */
class RoadSegmentArc : public RoadSegment
{
public:
	/* 
		The center is constrained by the line which passes through the midpoint of the line segment between
		the start and end points and which is perpendicular to this line segment. The radius of the circle is
		hence the distance from center to both start and end.
	*/
	RoadSegmentArc(const glm::vec2& center, const glm::vec2& start, const glm::vec2& end);

	glm::vec2 get_position(float t) const;
	glm::vec2 get_normal(float t) const;
	glm::vec2 get_tangent(float t) const;
	float get_length(float t) const;
	float get_parameter_at_distance(float distance) const;
private:
	glm::vec2 center;
	float radius;
	float start_angle;
	float end_angle;
	float sign;
};
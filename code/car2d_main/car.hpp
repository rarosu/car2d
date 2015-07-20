#pragma once

#include <vector>
#include <yaml-cpp/yaml.h>
#define NOMINMAX
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include "input.hpp"
#include "config.hpp"

class Car
{
public:
	Car(const YAML::Node& car_config, const YAML::Node& config);

	void update(float dt, const InputState& input_state_current, const InputState& input_state_previous);
	void render(float dt, float interpolation);

	const glm::vec2& get_position() const;
private:
	const YAML::Node& config;
	Controls controls;
	
	struct Static
	{
		float mass;								// The mass of the car (kg)
		float inertia;							// The moment of inertia of the car (kg * m^2)
		std::vector<glm::vec2> torque_curve;	// The torque curve of the engine (rpm -> N*m)
		std::vector<glm::vec2> slip_curve;		// The slip curve of the wheels (N/A -> N/A)
		float cg_to_front;						// The distance from center of gravity to front (m)
		float cg_to_back;						// The distance from center of gravity to back (m)
		float cg_height;						// The distance from center of gravity to ground (m)
		float halfwidth;						// The half width of the car (m)
		float cg_to_front_axle;					// The distance from center of gravity to front axle (m)
		float cg_to_back_axle;					// The distance from center of gravity to back axle (m)
		float wheel_radius;						// The radius of the wheels (m)
		float wheel_width;						// The width of the wheels (m)
	} description;

	glm::vec2 position;							// The position of the car relative to world origin (m)
	glm::vec2 velocity;							// The velocity of the entire car (m/s)
	glm::vec2 acceleration;						// The acceleration of the entire car (m/s^2)
	float orientation;							// The orientation of the car (rad)

	PerInstance uniform_instance_data;
	GLuint mesh_vs;
	GLuint mesh_fs;
	GLuint mesh_program;
	GLuint quad_position_vbo;
	GLuint quad_vao;
	GLuint uniform_instance_buffer;

	Car(const Car&);
	Car& operator=(const Car&);
};
#pragma once

#include <vector>
#include <yaml-cpp/yaml.h>
#define NOMINMAX
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include "input.hpp"
#include "config.hpp"
#include "stats.hpp"
#include <fstream>

class Car
{
public:
	Car(const YAML::Node& car_config, const YAML::Node& config, Stats& stats);
	~Car();

	void update(float dt, const InputState& input_state_current, const InputState& input_state_previous);
	void render(float dt, float interpolation);

	const glm::vec2& get_position() const;
	const glm::vec2& get_facing() const;
	const glm::vec2& get_velocity() const;
	const glm::vec2& get_acceleration() const;
private:
	static const float RPM_TO_ANGULAR_VELOCITY;
	static const float ANGULAR_VELOCITY_TO_RPM;
	static const float G;
	static const float EPSILON;

	const YAML::Node& config;
	Stats& stats;
	Controls controls;
	
	struct Static
	{
		// Configuration values.
		float mass;									// The mass of the car (kg)
		std::vector<glm::vec2> torque_curve;		// The torque curve of the engine (rpm -> N*m)
		std::vector<glm::vec2> slip_curve;			// The slip curve of the wheels (N/A -> N/A)
		float cornering_stiffness;					// The cornering stiffness of the wheels (N/A)
		float tire_grip;							// The maximum amount of grip of the wheels (N/A)
		float lock_grip_factor;						// Rear wheel grip factor (N/A)
		std::vector<float> gear_ratios;				// The transmission ratios for the gears (0 is reverse) (N/A)
		float differential_ratio;					// The transmission ratio for the differential (N/A)
		float transmission_efficiency;				// The percentage of remaining energy after transmission (N/A)
		float cg_to_front;							// The distance from center of gravity to front (m)
		float cg_to_back;							// The distance from center of gravity to back (m)
		float cg_height;							// The distance from center of gravity to ground (m)
		float height;								// The from the ground to the top of the car (m)
		float halfwidth;							// The half width of the car (m)
		float cg_to_front_axle;						// The distance from center of gravity to front axle (m)
		float cg_to_back_axle;						// The distance from center of gravity to back axle (m)
		float drag_coefficient;						// The C_d coefficient in the drag equation (N/A)
		float wheel_mass;							// The mass of a single wheel (kg)
		float wheel_radius;							// The radius of the wheels (m)
		float wheel_width;							// The width of the wheels (m)
		float wheel_max_friction;					// What even is this..? (N/A)
		float wheel_rolling_friction;				// The rolling friction coefficient of the wheels (N/A)
		float max_steer_angle;						// The maximum angle the wheels can be at relative to the car (rad)

		float air_density;							// The density of the surrounding air (kg/m^3)

		// Inferred values.
		float inertia;								// The moment of inertia of the car (kg * m^2)
		float wheel_inertia;						// The moment of inertia of a single wheel (kg * m^2)
	} description;

	float orientation;							// The orientation of the car relative to world orientation (rad)
	float car_angular_velocity;					// The current rate of turn for the car (change in yaw) (rad/s)
	float steer_angle;							// The orientation of the front wheels relative to car orientation (rad)
	float wheel_angular_velocity;				// The rear wheel angular velocity (rad/s)
	glm::vec2 position;							// The position of the car relative to world origin (m)
	glm::vec2 velocity;							// The velocity of the car relative to world orientation (m/s)
	glm::vec2 acceleration;						// The acceleration of the car relative to world orientation (m/s^2)
	glm::vec2 facing;							// The orientation of the longitudal axis relative to the world orienation (calculated from orientation) (N/A)
	glm::vec2 velocity_local;					// The velocity of the car relative to car orientation (m/s)
	glm::vec2 acceleration_local;				// The acceleration of the car relative to car orientation (m/s^2)
	bool throttle;								// Whether the throttle is active or not.
	bool reverse;								// Whether the reverse/brake is active or not.
	bool ebrake;								// Whether the parking brake is active or not.
	int gear;									// The index of the current gear in interval [0, 5]

	PerInstance uniform_instance_data;
	GLuint mesh_vs;
	GLuint mesh_fs;
	GLuint mesh_program;
	GLuint quad_position_vbo;
	GLuint quad_vao;
	GLuint uniform_instance_buffer;

	int frame_count;
	std::fstream stat_file;

	Car(const Car&);
	Car& operator=(const Car&);

	void handle_input(const InputState& input_state_current, const InputState& input_state_previous);
	void update_physics(float dt);
	float lerp_curve(const std::vector<glm::vec2>& curve, float x) const;
};
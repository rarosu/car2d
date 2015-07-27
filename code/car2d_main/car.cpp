#include "car.hpp"
#include "shader.hpp"
#include "debug.hpp"
#include <glm/gtx/compatibility.hpp>

const float Car::RPM_TO_ANGULAR_VELOCITY = 2.0f * glm::pi<float>() / 60.0f;
const float Car::ANGULAR_VELOCITY_TO_RPM = 1.0f / RPM_TO_ANGULAR_VELOCITY;
const float Car::G = 9.82f;
const float Car::EPSILON = 10e-5f;

Car::Car(const YAML::Node& car_config, const YAML::Node& config)
	: config(config)
	, controls(config)
	, orientation(45.0f)
	, steer_angle(0.0f)
	, wheel_angular_velocity(0.0f)
	, facing(std::cos(orientation), std::sin(orientation))
	, throttle(false)
	, reverse(false)
	, ebrake(false)
	, gear(1)
{
	// Read all configuration values.
	description.mass = car_config["Mass"].as<float>();

	description.torque_curve.resize(car_config["TorqueCurve"].size());
	for (int i = 0; i < car_config["TorqueCurve"].size(); ++i)
	{
		description.torque_curve[i] = glm::vec2(car_config["TorqueCurve"][i][0].as<float>(), car_config["TorqueCurve"][i][1].as<float>());
	}

	description.slip_curve.resize(car_config["SlipCurve"].size());
	for (int i = 0; i < car_config["SlipCurve"].size(); ++i)
	{
		description.slip_curve[i] = glm::vec2(car_config["SlipCurve"][i][0].as<float>(), car_config["SlipCurve"][i][1].as<float>());
	}

	description.gear_ratios.resize(6);
	description.gear_ratios[0] = car_config["GearRatios"]["Reverse"].as<float>();
	description.gear_ratios[1] = car_config["GearRatios"]["First"].as<float>();
	description.gear_ratios[2] = car_config["GearRatios"]["Second"].as<float>();
	description.gear_ratios[3] = car_config["GearRatios"]["Third"].as<float>();
	description.gear_ratios[4] = car_config["GearRatios"]["Fourth"].as<float>();
	description.gear_ratios[5] = car_config["GearRatios"]["Fifth"].as<float>();
	description.differential_ratio = car_config["DifferentialRatio"].as<float>();
	description.transmission_efficiency = car_config["TransmissionEfficiency"].as<float>();

	description.cg_to_front = car_config["CGToFront"].as<float>();
	description.cg_to_back = car_config["CGToBack"].as<float>();
	description.cg_height = car_config["CGHeight"].as<float>();
	description.halfwidth = car_config["HalfWidth"].as<float>();
	description.cg_to_front_axle = car_config["CGToFrontAxle"].as<float>();
	description.cg_to_back_axle = car_config["CGToBackAxle"].as<float>();

	description.wheel_mass = car_config["WheelMass"].as<float>();
	description.wheel_radius = car_config["WheelRadius"].as<float>();
	description.wheel_width = car_config["WheelWidth"].as<float>();
	description.wheel_friction = car_config["WheelFriction"].as<float>();
	description.max_steer_angle = car_config["MaxSteerAngle"].as<float>();

	// Add the wheel mass to the total car mass.
	description.mass += 4 * description.wheel_mass;

	// Calculate the moment of inertia for a cuboid (car body).
	float length = description.cg_to_front + description.cg_to_back;
	float width = 2.0f * description.halfwidth;
	description.inertia = (1.0f / 12.0f) * description.mass * (length * length + width * width);

	// Calculate the moment of inertia for a cylinder (wheel).
	description.wheel_inertia = 0.5f * description.wheel_mass * (description.wheel_radius * description.wheel_radius);

	// Setup the rendering.
	glm::vec2 positions[] = { glm::vec2(-0.5f, -0.5f), glm::vec2(-0.5f, 0.5f), glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f) };

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glGenBuffers(1, &quad_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	mesh_vs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_PLAIN2D_VS, GL_VERTEX_SHADER);
	mesh_fs = compile_shader_from_file(DIRECTORY_SHADERS + FILE_PLAIN2D_FS, GL_FRAGMENT_SHADER);
	mesh_program = glCreateProgram();
	glAttachShader(mesh_program, mesh_vs);
	glAttachShader(mesh_program, mesh_fs);
	link_program(mesh_program);

	uniform_instance_data.model_matrix = glm::mat3x4(glm::mat3(1));
	glGenBuffers(1, &uniform_instance_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstance), &uniform_instance_data, GL_DYNAMIC_DRAW);
}

void Car::update(float dt, const InputState& input_state_current, const InputState& input_state_previous)
{
	handle_input(input_state_current, input_state_previous);
	update_physics(dt);
}

void Car::handle_input(const InputState& input_state_current, const InputState& input_state_previous)
{
	// Handle throttle/braking
	throttle = true;
	//throttle = controls.is_pressed(controls.accelerate, input_state_current);
	reverse = controls.is_pressed(controls.reverse, input_state_current);
	ebrake = controls.is_pressed(controls.ebrake, input_state_current);

	// Handle steering.
	float steer_input = (int) controls.is_pressed(controls.left, input_state_current) - (int) controls.is_pressed(controls.right, input_state_current);
	steer_angle = steer_input * description.max_steer_angle;

	// TODO: Handle gearing.
}

void Car::update_physics(float dt)
{
	// Calculate the weight transfer with the current acceleration.
	float weight = description.mass * G;
	float wheelbase = description.cg_to_front_axle + description.cg_to_back_axle;
	float front_weight = (description.cg_to_back_axle / wheelbase) * weight - (description.cg_height / wheelbase) * description.mass * acceleration_local.x;
	float rear_weight = (description.cg_to_front_axle / wheelbase) * weight + (description.cg_height / wheelbase) * description.mass * acceleration_local.x;

	// Given the angular speed of the rear wheels, calculate engine speed and use that to look up maximum torque.
	// When the car is standing still, the engine is assumed to give the torque for the minimum defined speed value.
	// This is a simplification to avoid simulating a clutch.
	float transmission = description.gear_ratios[gear] * description.differential_ratio * description.transmission_efficiency;
	float engine_rpm = ANGULAR_VELOCITY_TO_RPM * wheel_angular_velocity / transmission;
	float engine_torque = throttle ? lerp_curve(description.torque_curve, engine_rpm) : 0.0f;

	// Calculate the torque exerted on the wheels. Also, integrate the angular velocity of the rear wheels!
	float wheel_torque = engine_torque * transmission;
	float wheel_angular_acceleration = wheel_torque / description.wheel_radius;
	wheel_angular_velocity += wheel_angular_acceleration * dt;

	// Calculate the force exerted on the road surface.
	float slip_ratio = (velocity_local.x >= EPSILON) ? (wheel_angular_velocity * description.wheel_radius - velocity_local.x) / velocity_local.x : wheel_angular_velocity * description.wheel_radius;
	float traction_force = lerp_curve(description.slip_curve, slip_ratio) * rear_weight;

	// Above a certain traction, the wheels lose grip and start spinning...
	traction_force = glm::min(traction_force, rear_weight * description.wheel_friction);

	// Sum the longitudal and latitudal forces.
	glm::vec2 force;
	force.x = traction_force;

	// Integrate!
	acceleration_local.x += traction_force / description.mass;
	velocity_local += acceleration_local * dt;
	
	// Turn the local quantities into world oriented quantities.
	float sn = std::sin(orientation);
	float cs = std::cos(orientation);

	acceleration.x = cs * acceleration_local.x + sn * acceleration_local.y;
	acceleration.y = -sn * acceleration_local.x + cs * acceleration_local.y;
	velocity.x = cs * velocity_local.x + sn * velocity_local.y;
	velocity.y = -sn * velocity_local.x + cs * velocity_local.y;

	// Integrate the position!
	position += velocity * dt;
}

void Car::render(float dt, float interpolation)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(mesh_program);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_INSTANCE_BINDING, uniform_instance_buffer);
	glBindVertexArray(quad_vao);
	
	// Update the transforms and the instance buffer.
	float sn = std::sin(orientation);
	float cs = std::cos(orientation);
	glm::mat3 rotation = glm::mat3(cs,  sn,  0,
								   -sn, cs,  0,
								   0,   0,   1);

	glm::vec2 interpolated_position = position + velocity * dt * interpolation;
	glm::mat3 translation = glm::mat3(1,					   0,						0,
									  0,					   1,						0,
									  interpolated_position.x, interpolated_position.y, 1);
	
	// Render the chassis.
	float width = 2.0f * description.halfwidth;
	float length = description.cg_to_back + description.cg_to_front;
	glm::mat3 scale = glm::mat3(length, 0,	   0,
								0,	    width, 0,
								0,	    0,	   1);

	uniform_instance_data.model_matrix = glm::mat3x4(translation * rotation * scale);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerInstance), &uniform_instance_data);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	


	// Render the wheels.
	glm::mat3 scale_wheel = glm::mat3(2.0f * description.wheel_radius, 0,						0,
									  0,							   description.wheel_width, 0,
									  0,							   0,						1);
	glm::vec2 offsets[] = { glm::vec2(description.cg_to_front_axle, description.halfwidth),
							glm::vec2(description.cg_to_front_axle, -description.halfwidth), 
							glm::vec2(-description.cg_to_back_axle, description.halfwidth),
							glm::vec2(-description.cg_to_back_axle, -description.halfwidth) };

	float rotations[] = { steer_angle, steer_angle, 0, 0 };

	for (int i = 0; i < 4; ++i)
	{
		glm::mat3 translation_wheel = glm::mat3(1, 0, 0,
												0, 1, 0,
												offsets[i][0], offsets[i][1], 1);
	
		float sn_wheel = std::sin(rotations[i]);
		float cs_wheel = std::cos(rotations[i]);
		glm::mat3 rotation_wheel = glm::mat3(cs_wheel,  sn_wheel, 0,
											 -sn_wheel, cs_wheel, 0,
											 0,			0,		  1);

		uniform_instance_data.model_matrix = glm::mat3x4(translation * rotation * translation_wheel * rotation_wheel * scale_wheel);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerInstance), &uniform_instance_data);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

const glm::vec2& Car::get_position() const
{
	return position;
}

const glm::vec2& Car::get_facing() const
{
	return facing;
}

float Car::lerp_curve(const std::vector<glm::vec2>& curve, float x) const
{
	if (curve.size() == 0) return 0.0f;

	float x1 = curve.front().x;
	float x2 = curve.back().x;

	if (x < x1) return curve.front().y;
	if (x > x2) return curve.back().y;

	int i = 0;
	for (; i < static_cast<int>(curve.size()); ++i) 
	{
		if (curve[i].x > x)
			break;
	}

	float dx = curve[i].x - curve[i - 1].x;
	float dy = curve[i].y - curve[i - 1].y;

	return curve[i - 1].y + ((x - curve[i - 1].x) / dx) * dy;
}
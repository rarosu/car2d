#include "car.hpp"
#include "shader.hpp"
#include "debug.hpp"
#include <glm/gtx/compatibility.hpp>

const float Car::RPM_TO_ANGULAR_VELOCITY = 2.0f * glm::pi<float>() / 60.0f;
const float Car::ANGULAR_VELOCITY_TO_RPM = 1.0f / RPM_TO_ANGULAR_VELOCITY;
const float Car::G = 9.82f;
const float Car::EPSILON = 10e-5f;

Car::Car(const YAML::Node& car_config, const YAML::Node& config, Stats& stats)
	: config(config)
	, stats(stats)
	, controls(config)
	, orientation(0.0f * DEGREES_TO_RADIANS)
	, steer_angle(0.0f)
	, wheel_angular_velocity(0.0f)
	, facing(std::cos(orientation), std::sin(orientation))
	, throttle(false)
	, reverse(false)
	, ebrake(false)
	, gear(1)
	, frame_count(0)
	, stat_file("stat.txt", std::ios_base::out | std::ios_base::trunc)
{
	//throttle = true;
	stat_file << "throttle" << ";" 
		<< "engine_rpm" << ";" 
		<< "engine_torque" << ";" 
		<< "drive_torque" << ";" 
		<< "drive_angular_acceleration" << ";" 
		<< "drive_angular_velocity" << ";"
	    << "slip_ratio" << ";" 
		<< "traction_force" << ";"
		<< "wheel_torque" << ";" 
		<< "wheel_angular_acceleration" << ";" 
		<< "wheel_angular_velocity" << ";" 
		<< "acceleration_local.x" << ";" 
		<< "velocity_local.x" << std::endl;

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
	description.height = car_config["Height"].as<float>();
	description.halfwidth = car_config["HalfWidth"].as<float>();
	description.cg_to_front_axle = car_config["CGToFrontAxle"].as<float>();
	description.cg_to_back_axle = car_config["CGToBackAxle"].as<float>();
	description.drag_coefficient = car_config["DragCoefficient"].as<float>();

	description.wheel_mass = car_config["WheelMass"].as<float>();
	description.wheel_radius = car_config["WheelRadius"].as<float>();
	description.wheel_width = car_config["WheelWidth"].as<float>();
	description.wheel_max_friction = car_config["WheelMaxFriction"].as<float>();
	description.wheel_rolling_friction = car_config["WheelRollingFriction"].as<float>();
	description.max_steer_angle = car_config["MaxSteerAngle"].as<float>();

	description.air_density = config["World"]["AirDensity"].as<float>();

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

Car::~Car()
{
	stat_file.close();
}

void Car::update(float dt, const InputState& input_state_current, const InputState& input_state_previous)
{
	handle_input(input_state_current, input_state_previous);
	update_physics(dt);
}

void Car::handle_input(const InputState& input_state_current, const InputState& input_state_previous)
{
	// Handle throttle/braking
	//throttle = true;
	throttle = controls.is_pressed(controls.accelerate, input_state_current);
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
	
	// For testing, half weight front, half weight back.
	//float weight = description.mass * G;
	//float front_weight = weight * 0.5f;
	//float rear_weight = weight * 0.5f;

	// Given the angular speed of the rear wheels, calculate engine speed and use that to look up maximum torque.
	// When the car is standing still, the engine is assumed to give the torque for the minimum defined speed value.
	// This is a simplification to avoid simulating a clutch.
	float transmission = description.gear_ratios[gear] * description.differential_ratio * description.transmission_efficiency;
	float engine_rpm = ANGULAR_VELOCITY_TO_RPM * wheel_angular_velocity * transmission;
	float engine_torque = throttle ? lerp_curve(description.torque_curve, engine_rpm) : 0.0f;
	
	// Calculate the torque exerted on the wheels. Also, integrate the angular velocity of the rear wheels!
	float drive_torque = engine_torque * transmission;
	float drive_angular_acceleration = drive_torque / description.wheel_inertia;
	wheel_angular_velocity += drive_angular_acceleration * dt;
	float drive_angular_velocity = wheel_angular_velocity;
	
	// Calculate the force exerted on the road surface.
	float slip_ratio = (wheel_angular_velocity * description.wheel_radius - velocity_local.x) / (std::abs(velocity_local.x) + 1);
	float traction_force = lerp_curve(description.slip_curve, slip_ratio) * rear_weight;

	// Calculate the rolling friction.
	float rolling_friction_force = description.wheel_rolling_friction * velocity_local.x;

	// Calculate the opposing torque on the wheels.
	float wheel_force = -traction_force - rolling_friction_force;
	float wheel_torque = wheel_force * description.wheel_radius;
	float wheel_angular_acceleration = wheel_torque / description.wheel_inertia;
	wheel_angular_velocity += wheel_angular_acceleration * dt;

	// Calculate the drag force.
	float speed = glm::length(velocity_local);
	float frontal_area = 2.0f * description.halfwidth * description.height;
	glm::vec2 drag_force = -0.5f * description.air_density * description.drag_coefficient * frontal_area * speed * velocity_local;
	//glm::vec2 drag_force;

	// Sum the longitudal and latitudal forces.

	glm::vec2 force;
	force.x = traction_force + drag_force.x;
	force.y = drag_force.y;
	
	// Calculate the opposing torque on the wheels.
	//float traction_torque = -traction_force * description.wheel_radius;
	//float drag_torque = -drag_force.x * description.wheel_radius;
	//float wheel_torque = traction_torque + drag_torque;
	//float wheel_angular_acceleration = wheel_torque / description.wheel_inertia;
	//wheel_angular_velocity += wheel_angular_acceleration * dt;
	
	// Integrate!
	acceleration_local.x += force.x / description.mass;
	velocity_local += acceleration_local * dt;
	
	// Turn the local quantities into world oriented quantities.
	float sn = std::sin(orientation);
	float cs = std::cos(orientation);
	
	acceleration.x = cs * acceleration_local.x - sn * acceleration_local.y;
	acceleration.y = sn * acceleration_local.x + cs * acceleration_local.y;
	velocity.x = cs * velocity_local.x - sn * velocity_local.y;
	velocity.y = sn * velocity_local.x + cs * velocity_local.y;
	
	// Integrate the position!
	position += velocity * dt;
	
	// Update the statistics.
	stats.append_update_line("position", "Position: (%.1f, %.1f)", position.x, position.y);
	stats.append_update_line("velocity x", "Longitudal velocity: %.1f m/s", velocity_local.x);
	stats.append_update_line("acceleration x", "Longitudal acceleration: %.1f m/s^2", acceleration_local.x);
	stats.append_update_line("engine rpm", "Engine RPM: %.1f", engine_rpm);
	stats.append_update_line("engine torque", "Engine torque: %.1f Nm", engine_torque);
	stats.append_update_line("slip ratio", "Slip ratio: %.1f", slip_ratio);
	stats.append_update_line("traction force", "Traction force: %.1f N", traction_force);
	stats.append_update_line("drag force x", "Longitudal drag force: %.1f N", drag_force.x);
	stats.append_update_line("drive torque", "Drive torque: %.1f Nm", drive_torque);
	stats.append_update_line("wheel torque", "Wheel torque: %.1f Nm", wheel_torque);

	frame_count++;
	stat_file << throttle << ";"
		<< engine_rpm << ";"
		<< engine_torque << ";"
		<< drive_torque << ";"
		<< drive_angular_acceleration << ";"
		<< drive_angular_velocity << ";"
		<< slip_ratio << ";"
		<< traction_force << ";"
		<< wheel_torque << ";"
		<< wheel_angular_acceleration << ";"
		<< wheel_angular_velocity << ";"
		<< acceleration_local.x << ";" 
		<< velocity_local.x << std::endl;
	if (frame_count > 40)
		throttle = false;

	//stats.append_update_line("position", "Position: (%.1f, %.1f)", position.x, position.y);
	//stats.append_update_line("speed", "Speed: %.1f m/s", glm::length(velocity));
	//stats.append_update_line("engine rpm", "Engine RPM: %.1f", engine_rpm);
	//stats.append_update_line("acceleration", "Acceleration: %.1f m/s^2", glm::length(acceleration));
	//stats.append_update_line("front weight", "Front weight: %.1f kg", front_weight);
	//stats.append_update_line("rear weight", "Rear weight: %.1f kg", rear_weight);
	//stats.append_update_line("traction force", "Traction force: %.1f N", traction_force);
	//stats.append_update_line("traction torque", "Traction torque: %.1f Nm", traction_torque);
	//stats.append_update_line("drag force", "Drag force: %.1f N", glm::length(drag_force));
	//stats.append_update_line("drag torque", "Drag torque: %.1f Nm", drag_torque);
	//stats.append_update_line("engine torque", "Engine torque: %.1f Nm", engine_torque);
	//stats.append_update_line("drive torque", "Drive torque: %.1f Nm", drive_torque);
	//stats.append_update_line("drive angular acceleration", "Drive angular acceleration: %.1f rad/s^2", drive_angular_acceleration);
	//stats.append_update_line("slip ratio", "Slip ratio: (%.1f - %.1f) / (%.1f + 1) = %.3f", wheel_angular_velocity * description.wheel_radius, velocity_local.x, velocity_local.x, slip_ratio);
	//stats.append_update_line("wheel torque", "Wheel torque: %.1f Nm", wheel_torque);
	//stats.append_update_line("wheel angular acceleration", "Wheel angular acceleration: %.1f rad/s^2", wheel_angular_acceleration);
	//stats.append_update_line("wheel angular velocity", "Wheel angular velocity: %.1f rad/s", wheel_angular_velocity);
	////stats.append_update_line("traction angular acceleration", "Traction angular acceleration: %f rad/s^2", traction_angular_acceleration);
	//stats.append_update_line("throttle", "Throttle: %d", throttle);

	//float weight = description.mass * G;
	////float wheelbase = description.cg_to_front_axle + description.cg_to_back_axle;
	////float front_weight = (description.cg_to_back_axle / wheelbase) * weight - (description.cg_height / wheelbase) * description.mass * acceleration_local.x;
	////float rear_weight = (description.cg_to_front_axle / wheelbase) * weight + (description.cg_height / wheelbase) * description.mass * acceleration_local.x;
	//float front_weight = weight * 0.5f;
	//float rear_weight = weight * 0.5f;
	//
	//float transmission = description.gear_ratios[gear] * description.differential_ratio;
	//float engine_rpm = ANGULAR_VELOCITY_TO_RPM * wheel_angular_velocity * transmission;
	//float engine_torque = lerp_curve(description.torque_curve, engine_rpm);
	//
	//float drive_torque = engine_torque * transmission * description.transmission_efficiency;
	//float drive_angular_acceleration = drive_torque / description.wheel_inertia;
	//wheel_angular_velocity += drive_angular_acceleration * dt;
	//
	//float wheel_slip = (wheel_angular_velocity - velocity_local.x / description.wheel_radius) / glm::max(wheel_angular_velocity, velocity_local.x / description.wheel_radius);
	//float adhesion = lerp_curve(description.slip_curve, wheel_slip);
	//float traction_force = adhesion * rear_weight;
	//float traction_torque = traction_force * description.wheel_radius;
	//
	//float wheel_angular_acceleration = -traction_torque / description.wheel_inertia;
	//wheel_angular_velocity += wheel_angular_acceleration * dt;
	//
	//acceleration_local.x += traction_force / description.mass;
	//velocity_local += acceleration_local * dt;
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

	//glm::vec2 interpolated_position = position + velocity * dt * interpolation;
	glm::vec2 interpolated_position = position;
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

const glm::vec2& Car::get_velocity() const
{
	return velocity;
}

const glm::vec2& Car::get_acceleration() const
{
	return acceleration;
}

float Car::lerp_curve(const std::vector<glm::vec2>& curve, float x) const
{
	if (curve.size() == 0) return 0.0f;

	float x1 = curve.front().x;
	float x2 = curve.back().x;

	if (x <= x1) return curve.front().y;
	if (x >= x2) return curve.back().y;

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
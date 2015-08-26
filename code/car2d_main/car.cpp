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
	, car_angular_velocity(0.0f)
	, steer_angle(0.0f)
	, wheel_angular_velocity(0.0f)
	, facing(std::cos(orientation), std::sin(orientation))
	, throttle(false)
	, reverse(false)
	, ebrake(false)
	, gear(1)
	, automatic(true)
	, stat_file("stats.csv", ",")
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

	description.cornering_stiffness = car_config["CorneringStiffness"].as<float>();
	description.tire_grip = car_config["TireGrip"].as<float>();
	description.lock_grip_factor = car_config["LockGripFactor"].as<float>();

	description.gear_ratios.resize(6);
	description.gear_ratios[0] = car_config["GearRatios"]["Reverse"].as<float>();
	description.gear_ratios[1] = car_config["GearRatios"]["First"].as<float>();
	description.gear_ratios[2] = car_config["GearRatios"]["Second"].as<float>();
	description.gear_ratios[3] = car_config["GearRatios"]["Third"].as<float>();
	description.gear_ratios[4] = car_config["GearRatios"]["Fourth"].as<float>();
	description.gear_ratios[5] = car_config["GearRatios"]["Fifth"].as<float>();
	description.differential_ratio = car_config["DifferentialRatio"].as<float>();
	description.transmission_efficiency = car_config["TransmissionEfficiency"].as<float>();
	description.gear_down_rpm = car_config["GearUpRPM"].as<float>();
	description.gear_up_rpm = car_config["GearDownRPM"].as<float>();

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
	description.max_steer_angle = car_config["MaxSteerAngle"].as<float>() * DEGREES_TO_RADIANS;

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

	//velocity_local.y = 1.0f;
}

Car::~Car()
{

}

void Car::update(float dt)
{
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

	// Handle gearing.
	if (controls.is_clicked(controls.toggle_automatic, input_state_current, input_state_previous))
		automatic = !automatic;

	if (!automatic)
	{
		if (controls.is_clicked(controls.gear_up, input_state_current, input_state_previous))
		{
			gear = glm::clamp(gear + 1, 1, 5);
		}

		if (controls.is_clicked(controls.gear_down, input_state_current, input_state_previous))
		{
			gear = glm::clamp(gear - 1, 1, 5);
		}
	}
}

void Car::update_physics(float dt)
{
	// Prevent low speed madness.
	float speed = glm::length(velocity_local);
	if (speed < 1.0f)
	{
		if (throttle)
		{
			velocity_local.x = 1.0f;
			speed = 1.0f;
			wheel_angular_velocity = speed / description.wheel_radius;
		}
		else
		{
			acceleration_local = glm::vec2();
			velocity_local = glm::vec2();
			car_angular_velocity = 0;
			wheel_angular_velocity = 0;
			speed = 0.0f;
		}
	}

	// Calculate weight distribution.
	float weight = description.mass * G;
	float wheelbase = description.cg_to_front_axle + description.cg_to_back_axle;
	float front_weight = (description.cg_to_back_axle / wheelbase) * weight - (description.cg_height / wheelbase) * description.mass * acceleration_local.x;
	float rear_weight = (description.cg_to_front_axle / wheelbase) * weight + (description.cg_height / wheelbase) * description.mass * acceleration_local.x;

	// Given the angular speed of the rear wheels, calculate engine speed and use that to look up maximum torque.
	// When the car is standing still, the engine is assumed to give the torque for the minimum defined speed value.
	// This is a simplification to avoid simulating a clutch.
	float transmission = description.gear_ratios[gear] * description.differential_ratio * description.transmission_efficiency;
	float engine_rpm = ANGULAR_VELOCITY_TO_RPM * wheel_angular_velocity * transmission;

	if (automatic)
	{
		if (engine_rpm >= description.gear_up_rpm)
		{
			gear = glm::clamp(gear + 1, 1, 5);
		}
	
		if (engine_rpm <= description.gear_down_rpm)
		{
			gear = glm::clamp(gear - 1, 1, 5);
		}
	
		transmission = description.gear_ratios[gear] * description.differential_ratio * description.transmission_efficiency;
		engine_rpm = ANGULAR_VELOCITY_TO_RPM * wheel_angular_velocity * transmission;
	}

	float engine_torque = throttle ? lerp_curve(description.torque_curve, engine_rpm) : 0.0f;

	// Calculate the torque exerted on the wheels. Also, integrate the angular velocity of the rear wheels!
	float drive_torque = engine_torque * transmission;
	float drive_angular_acceleration = drive_torque / description.wheel_inertia;
	wheel_angular_velocity += drive_angular_acceleration * dt;
	float prefriction_wheel_angular_velocity = wheel_angular_velocity;

	// Calculate the force exerted on the road surface.	
	float slip_ratio = velocity_local.x < 0.01 ? 0 : (wheel_angular_velocity * description.wheel_radius - velocity_local.x) / std::abs(velocity_local.x);
	float traction_force = lerp_curve(description.slip_curve, slip_ratio) * rear_weight;

	// Calculate the lateral slip angles and determine the lateral cornering force.
	float front_angular_velocity = car_angular_velocity * description.cg_to_front_axle;
	float rear_angular_velocity = -car_angular_velocity * description.cg_to_back_axle;
	float slip_angle_front = std::atan2(velocity_local.y + front_angular_velocity, std::abs(velocity_local.x)) - glm::sign(velocity_local.x) * steer_angle;
	float slip_angle_rear  = std::atan2(velocity_local.y + rear_angular_velocity,  std::abs(velocity_local.x));
	
	//float rear_grip = description.tire_grip * (ebrake ? description.lock_grip_factor : 1.0f);
	//float cornering_force_front = front_weight * glm::clamp(-description.cornering_stiffness * slip_angle_front, -description.tire_grip, description.tire_grip);
	//float cornering_force_rear = rear_weight * glm::clamp(-description.cornering_stiffness * slip_angle_rear, -rear_grip, rear_grip);
	float cornering_force_front = front_weight * -description.cornering_stiffness * slip_angle_front;
	float cornering_force_rear = rear_weight * -description.cornering_stiffness * slip_angle_rear;

	// Make sure the amount of traction on the drive wheels stay within the circle of traction.
	float traction_circle_radius = 1.2f;
	glm::vec2 total_traction = glm::vec2(traction_force, cornering_force_rear) / rear_weight;
	if (total_traction.x * total_traction.x + total_traction.y * total_traction.y >= traction_circle_radius * traction_circle_radius)
	{
		total_traction = glm::normalize(total_traction);
		total_traction *= traction_circle_radius;

		traction_force = total_traction.x;
		cornering_force_rear = total_traction.y;
	}

	// Calculate the torque on the car body and integrate car yaw rate and orientation.
	float cornering_torque_front = cornering_force_front * description.cg_to_front_axle;
	float cornering_torque_rear = cornering_force_rear * description.cg_to_back_axle;
	float car_torque = std::cos(steer_angle) * cornering_torque_front - cornering_torque_rear;

	float car_angular_acceleration = car_torque / description.inertia;
	car_angular_velocity += car_angular_acceleration * dt;
	orientation += car_angular_velocity * dt;
	
	// Calculate the wind drag force on the car. Simplification that the area facing the velocity direction is the front.
	float area = description.height * 2.0f * description.halfwidth;
	float drag_multiplier = 0.5f * description.air_density * area * description.drag_coefficient;
	glm::vec2 drag_force = -drag_multiplier * speed * velocity_local;

	// Calculate the rolling friction force on the car.
	//glm::vec2 rolling_friction_force = -description.wheel_rolling_friction * velocity_local;
	float rolling_friction_force = -30.0f * drag_multiplier * velocity_local.x;

	// Calculate the opposing torque on the wheels. Includes braking.
	float braking_torque = reverse ? -3000 : 0;
	float wheel_friction_force = -traction_force;
	float wheel_friction_torque = wheel_friction_force * description.wheel_radius + braking_torque * glm::sign(wheel_angular_velocity);
	float wheel_friction_angular_acceleration = wheel_friction_torque / description.wheel_inertia;
	wheel_angular_velocity += wheel_friction_angular_acceleration * dt;

	// Sum the forces on the car's CG and integrate the velocity.
	glm::vec2 force = drag_force;
	force.x += traction_force + rolling_friction_force;
	force.y += std::cos(steer_angle) * cornering_force_front + cornering_force_rear;

	acceleration_local = force / description.mass;
	velocity_local += acceleration_local * dt;
	
	// Calculate the acceleration and velocity in world coordinates and integrate world position.
	float sn = std::sin(orientation);
	float cs = std::cos(orientation);

	acceleration.x = cs * acceleration_local.x - sn * acceleration_local.y;
	acceleration.y = sn * acceleration_local.x + cs * acceleration_local.y;
	velocity.x = cs * velocity_local.x - sn * velocity_local.y;
	velocity.y = sn * velocity_local.x + cs * velocity_local.y;

	position += velocity * dt;

	// Update statistics //
	
	// Statistics for debugging steering //
	//stats.append_update_line("position", "Position (%f, %f)", position.x, position.y);
	//stats.append_update_line("velocity", "Velocity Local (%f, %f)", velocity_local.x, velocity_local.y);
	//stats.append_update_line("acceleration", "Acceleration Local (%f, %f)", acceleration_local.x, acceleration_local.y);
	//stats.append_update_line("steer", "Steer: %f", steer_angle * RADIANS_TO_DEGREES);
	//stats.append_update_line("yaw rate front", "Yaw Rate Front: %f m/s", front_angular_velocity);
	//stats.append_update_line("yaw rate rear", "Yaw Rate Rear: %f m/s", rear_angular_velocity);
	//stats.append_update_line("slip angle front", "Slip Angle Front: %f", slip_angle_front * RADIANS_TO_DEGREES);
	//stats.append_update_line("slip angle rear", "Slip Angle Rear: %f", slip_angle_rear * RADIANS_TO_DEGREES);
	//stats.append_update_line("cornering torque front", "Cornering Torque Front: %f Nm", cornering_torque_front);
	//stats.append_update_line("cornering torque rear", "Cornering Torque Rear: %f Nm", cornering_torque_rear);
	//stats.append_update_line("cornering force front", "Cornering Force Front: %f N", cornering_force_front);
	//stats.append_update_line("cornering force rear", "Cornering Force Rear: %f N", cornering_force_rear);

	// Statistics for debugging traction and braking //
	stats.append_update_line("position", "Position (%f, %f)", position.x, position.y);
	stats.append_update_line("speed", "Speed: %f km/h", glm::length(velocity_local) * 3.6);
	stats.append_update_line("speedms", "Speed: %f m/s", glm::length(velocity_local));
	stats.append_update_line("acceleration", "Acceleration Local (%f, %f)", acceleration_local.x, acceleration_local.y);
	stats.append_update_line("rpm", "Engine RPM: %d rev/min", (int) engine_rpm);
	stats.append_update_line("gear", "Current gear: %d", gear);
	stats.append_update_line("automatic", "Automatic: %d", automatic);
	stats.append_update_line("engine torque", "Engine torque: %f Nm", engine_torque);
	stats.append_update_line("drive torque", "Drive torque: %f Nm", drive_torque);
	stats.append_update_line("wheel rate", "Wheel angular velocity: %f rad/s", prefriction_wheel_angular_velocity);
	stats.append_update_line("slip ratio", "Slip ratio: %f", slip_ratio);
	stats.append_update_line("traction force", "Traction force: %f N", traction_force);
	stats.append_update_line("drag force", "Drag force: %f N", glm::length(drag_force));
	stats.append_update_line("rolling friction", "Rolling friction: %f N", glm::abs(rolling_friction_force));

	stat_file.add_column("Throttle", throttle);
	stat_file.add_column("Speed m/s", glm::length(velocity_local));
	stat_file.add_column("Acceleration", acceleration_local.x);
	stat_file.add_column("Engine RPM", engine_rpm);
	stat_file.add_column("Engine Torque", engine_torque);
	stat_file.add_column("Drive Torque", drive_torque);
	stat_file.add_column("Pre-Friction Wheel Rate", prefriction_wheel_angular_velocity);
	stat_file.add_column("Post-Friction Wheel Rate", wheel_angular_velocity);
	stat_file.add_column("Slip Ratio", slip_ratio);
	stat_file.add_column("Traction Force", traction_force);
	stat_file.add_column("Drag Force", glm::length(drag_force));
	stat_file.add_column("Rolling Friction", rolling_friction_force);
	stat_file.commit_line();
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
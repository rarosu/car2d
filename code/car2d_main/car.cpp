#include "car.hpp"
#include "shader.hpp"
#include "debug.hpp"

Car::Car(const YAML::Node& car_config, const YAML::Node& config)
	: config(config)
	, controls(config)
	, orientation(0.0f)
{
	// Read all configuration values.
	description.mass = car_config["Mass"].as<float>();
	description.cg_to_front = car_config["CGToFront"].as<float>();
	description.cg_to_back = car_config["CGToBack"].as<float>();
	description.cg_height = car_config["CGHeight"].as<float>();
	description.halfwidth = car_config["HalfWidth"].as<float>();
	description.cg_to_front_axle = car_config["CGToFrontAxle"].as<float>();
	description.cg_to_back_axle = car_config["CGToBackAxle"].as<float>();
	description.wheel_radius = car_config["WheelRadius"].as<float>();
	description.wheel_width = car_config["WheelWidth"].as<float>();
	
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

	// Calculate the moment of inertia for a cuboid.
	float length = description.cg_to_front + description.cg_to_back;
	float width = 2.0f * description.halfwidth;
	description.inertia = (1.0f / 12.0f) * description.mass * (length * length + width * width);

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

	// DEBUG: Put a bit of velocity on the car.
	//velocity.y = 1.0f;
	orientation = glm::radians(45.0f);
}

void Car::update(float dt, const InputState& input_state_current, const InputState& input_state_previous)
{
	velocity += acceleration * dt;
	position += velocity * dt;
	//std::cout << position << std::endl;
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
	glm::mat3 rotation = glm::mat3(cs, -sn, 0,
								   sn, cs,  0,
								   0,  0,   1);

	glm::vec2 interpolated_position = position + velocity * dt * interpolation;
	glm::mat3 translation = glm::mat3(1,					   0,						0,
									  0,					   1,						0,
									  interpolated_position.x, interpolated_position.y, 1);
	
	// Render the chassis.
	float width = 2.0f * description.halfwidth;
	float length = description.cg_to_back + description.cg_to_front;
	glm::mat3 scale = glm::mat3(width, 0,	   0,
								0,	   length, 0,
								0,	   0,	   1);

	uniform_instance_data.model_matrix = glm::mat3x4(translation * rotation * scale);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerInstance), &uniform_instance_data);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	


	// Render the wheels.
	glm::mat3 scale_wheel = glm::mat3(description.wheel_width, 0,								0,
									  0,					   2.0f * description.wheel_radius, 0,
									  0,					   0,								1);
	glm::vec2 offsets[] = { glm::vec2(description.halfwidth, description.cg_to_front_axle), 
							glm::vec2(-description.halfwidth, description.cg_to_front_axle), 
							glm::vec2(description.halfwidth, -description.cg_to_back_axle), 
							glm::vec2(-description.halfwidth, -description.cg_to_back_axle) };
	for (int i = 0; i < 4; ++i)
	{
		glm::mat3 translation_wheel = glm::mat3(1, 0, 0,
												0, 1, 0,
												offsets[i][0], offsets[i][1], 1);
	
		uniform_instance_data.model_matrix = glm::mat3x4(translation * rotation * translation_wheel * scale_wheel);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerInstance), &uniform_instance_data);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

const glm::vec2& Car::get_position() const
{
	return position;
}
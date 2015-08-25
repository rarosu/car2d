#include "main.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#define NOMINMAX
#include <GL/gl3w.h>
#include "debug.hpp"

int main(int argc, char* argv[])
{
	int result = 0;
	try
	{
		Car2DMain m;
		m.start();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
		std::cin.get();
		result = 1;
	}
	catch (...)
	{
		std::cerr << "Unrecognized exception caught at outmost level." << std::endl;
		std::cin.get();
		result = 1;
	}

	return result;
}

void __stdcall output_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cerr << message << std::endl;
	}
}

WindowContext::WindowContext(const YAML::Node& config, int viewport_width, int viewport_height)
	: window(nullptr)
	, glcontext(nullptr)
{
	// Setup SDL.
	if (SDL_Init(SDL_INIT_TIMER) != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
	}

	window = SDL_CreateWindow(config["Window"]["Title"].as<std::string>().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, viewport_width, viewport_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create window: ") + SDL_GetError());
	}

	// Initialize the OpenGL context.
	SDL_GLcontextFlag flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifndef NDEBUG
	flags = (SDL_GLcontextFlag)(flags | SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

	glcontext = SDL_GL_CreateContext(window);
	if (glcontext == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create OpenGL context: ") + SDL_GetError());
	}

	// Initialize the profile loader.
	if (gl3wInit() != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize gl3w"));
	}

	if (gl3wIsSupported(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR) != 1)
	{
		std::stringstream ss;
		ss << "OpenGL " << OPENGL_VERSION_MAJOR << "." << OPENGL_VERSION_MINOR << " is not supported.";
		throw std::runtime_error(ss.str());
	}

	// Setup an error callback function.
	glDebugMessageCallback(output_debug_message, nullptr);

	// Setup the initial OpenGL context state.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, viewport_width, viewport_height);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	{
		int major;
		int minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

		std::cout << "OpenGL version: " << major << "." << minor << std::endl;
	}

	// Set V-Sync enabled.
	SDL_GL_SetSwapInterval(1);
}

WindowContext::~WindowContext()
{
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
}

Car2DMain::Car2DMain()
	: config(YAML::LoadFile(PROJECT_ROOT + FILE_CONFIG))
	, zoom_level(config["Camera"]["ZoomLevel"].as<float>())
	, max_zoom_level(config["Camera"]["MaxZoomLevel"].as<float>())
	, min_zoom_level(config["Camera"]["MinZoomLevel"].as<float>())
	, viewport_width(config["Window"]["Width"].as<int>())
	, viewport_height(config["Window"]["Height"].as<int>())
	, running(true)
	, window_context(config, viewport_width, viewport_height)
	, ticker(DT, 5)
	, stats(viewport_width, viewport_height)
	, camera(Camera::create_projection(2.0f / zoom_level, viewport_width, viewport_height))
	, car(YAML::LoadFile(DIRECTORY_CARS + config["Assets"]["DefaultCar"].as<std::string>()), config, stats)
	, terrain(YAML::LoadFile(DIRECTORY_MAPS + config["Assets"]["DefaultMap"].as<std::string>()))
	, road(YAML::LoadFile(DIRECTORY_MAPS + config["Assets"]["DefaultMap"].as<std::string>()))
{
	setup_resources();
}

Car2DMain::~Car2DMain()
{
	
}

void Car2DMain::setup_resources()
{
	camera.set_origin(glm::vec2(0.0f, 15.0f));
	camera.set_facing(glm::vec2(0.0f, 1.0f));
	camera.recalculate_matrices();

	uniform_frame_data.view_matrix = glm::mat3x4(camera.get_view());
	uniform_frame_data.projection_matrix = glm::mat3x4(camera.get_projection());
	glGenBuffers(1, &uniform_frame_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_FRAME_BINDING, uniform_frame_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrame), &uniform_frame_data, GL_DYNAMIC_DRAW);
}

void Car2DMain::start()
{
	ticker.start();
	while (running)
	{
		ticker.tick();
		
		handle_events();
		while (ticker.poll_fixed_tick())
		{
			update(DT);
		}

		render(DT, ticker.get_interpolation());
	}
}

void Car2DMain::handle_events()
{
	input_state_previous = input_state_current;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				running = false;
			} break;

			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						viewport_width = event.window.data1;
						viewport_height = event.window.data2;
						glViewport(0, 0, viewport_width, viewport_height);
						camera.set_projection(Camera::create_projection(2.0f / zoom_level, viewport_width, viewport_height));
						stats.window_resized(viewport_width, viewport_height);
						std::cout << "Window resized to " << viewport_width << "x" << viewport_height << std::endl;
					} break;
				}
			} break;

			case SDL_KEYDOWN:
			{
				input_state_current.keys[event.key.keysym.scancode] = true;
			} break;

			case SDL_KEYUP:
			{
				input_state_current.keys[event.key.keysym.scancode] = false;
			} break;

			case SDL_MOUSEWHEEL:
			{
				zoom_level -= event.wheel.y;
				if (zoom_level > max_zoom_level)
					zoom_level = max_zoom_level;
				if (zoom_level < min_zoom_level)
					zoom_level = min_zoom_level;

				camera.set_projection(Camera::create_projection(2.0f / zoom_level, viewport_width, viewport_height));
			} break;
		}
	}

	car.handle_input(input_state_current, input_state_previous);
}

void Car2DMain::update(float dt)
{
	if (input_state_current.keys[SDL_SCANCODE_ESCAPE])
		running = false;

	// Update the car.
	car.update(dt);

	// Update the per frame buffer.
	//update_camera_free(dt);
	update_camera_chase();
	camera.recalculate_matrices();

	uniform_frame_data.view_matrix = glm::mat3x4(camera.get_view());
	uniform_frame_data.projection_matrix = glm::mat3x4(camera.get_projection());
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_FRAME_BINDING, uniform_frame_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerFrame), &uniform_frame_data);

	// Update the statistics.	
	stats.update_text();
}

void Car2DMain::update_camera_free(float dt)
{
	// Update a free-moving camera, controlled by the arrow keys.
	// For debug purposes only.
	const float CAMERA_SPEED = 10.0f;
	const float CAMERA_TURNSPEED = 3.14f;

	glm::vec2 origin = camera.get_origin();
	glm::vec2 facing = camera.get_facing();
	glm::vec2 right = camera.get_right();

	if (input_state_current.keys[SDL_SCANCODE_W] || input_state_current.keys[SDL_SCANCODE_UP])
		origin += facing * CAMERA_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_S] || input_state_current.keys[SDL_SCANCODE_DOWN])
		origin -= facing * CAMERA_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_D] || input_state_current.keys[SDL_SCANCODE_RIGHT])
		origin += right * CAMERA_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_A] || input_state_current.keys[SDL_SCANCODE_LEFT])
		origin -= right * CAMERA_SPEED * dt;
	camera.set_origin(origin);

	float angle = std::atan2(camera.get_facing().y, camera.get_facing().x);
	if (input_state_current.keys[SDL_SCANCODE_Q])
		angle -= CAMERA_TURNSPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_E])
		angle += CAMERA_TURNSPEED * dt;
	facing = glm::vec2(std::cos(angle), std::sin(angle));
	camera.set_facing(facing);
}

void Car2DMain::update_camera_chase()
{
	camera.set_origin(car.get_position());
	camera.set_facing(car.get_facing());
}

void Car2DMain::render(float dt, float interpolation)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_FRAME_BINDING, uniform_frame_buffer);

	terrain.render();
	road.render();
	car.render(dt, interpolation);
	stats.render();

	SDL_GL_SwapWindow(window_context.window);
}
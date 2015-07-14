#include "main.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <GL/gl3w.h>

int main(int argc, char* argv[])
{
	try
	{
		Car2DMain m;
		m.start();
	}
	catch (std::exception& e)
	{
		return 1;
	}

	std::cin.get();

    return 0;
}

void __stdcall output_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cerr << message << std::endl;
	}
}

InputState::InputState()
{
	memset(keys, 0, sizeof(keys));
}

Car2DMain::Car2DMain()
	: window(nullptr)
	, glcontext(nullptr)
	, running(true)
	, viewport_width(0)
	, viewport_height(0)
	, ticker(DT, 5)
{
	setup_context();
}

Car2DMain::~Car2DMain()
{
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
}

void Car2DMain::setup_context()
{
	// Setup SDL.
	if (SDL_Init(SDL_INIT_TIMER) != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
	}

	window = SDL_CreateWindow("Car2D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
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
	glViewport(0, 0, 800, 600);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);

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

void Car2DMain::start()
{
	Uint32 last_clock = SDL_GetTicks();

	ticker.start();
	while (running)
	{
		ticker.tick();
		
		handle_events();
		while (ticker.poll_fixed_tick())
		{
			Uint32 current_clock = SDL_GetTicks();
			Uint32 delta_clock = static_cast<Uint32>(current_clock - last_clock);
			last_clock = current_clock;

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
				switch (event.window.type)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						viewport_width = event.window.data1;
						viewport_height = event.window.data2;
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
		}
	}
}

void Car2DMain::update(float dt)
{
	
}

void Car2DMain::render(float dt, float interpolation)
{

}
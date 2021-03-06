#pragma once

#include <SDL2/SDL.h>
#include <yaml-cpp/yaml.h>
#include "config.hpp"
#include "ticker.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "car.hpp"
#include "road.hpp"
#include "terrain.hpp"
#include "stats.hpp"

class WindowContext
{
public:
	SDL_Window* window;
	SDL_GLContext glcontext;

	WindowContext(const YAML::Node& config, int viewport_width, int viewport_height);
	~WindowContext();
};

class Car2DMain
{
public:
	Car2DMain();
	~Car2DMain();

	void start();
private:
	YAML::Node config;
	float zoom_level;
	float max_zoom_level;
	float min_zoom_level;
	int viewport_width;
	int viewport_height;
	bool running;
	WindowContext window_context;
	InputState input_state_current;
	InputState input_state_previous;
	Ticker ticker;
	Camera camera;
	Car car;
	Road road;
	Terrain terrain;
	Stats stats;
	PerFrame uniform_frame_data;
	GLuint uniform_frame_buffer;

	void setup_resources();
	void handle_events();
	void update(float dt);
	void update_camera_free(float dt);
	void update_camera_chase();
	void render(float dt, float interpolation);
};
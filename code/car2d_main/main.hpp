#pragma once

#include <SDL2/SDL.h>
#include "ticker.hpp"

const float DT = 1.0f / 60.0f;
const int OPENGL_VERSION_MAJOR = 4;
const int OPENGL_VERSION_MINOR = 4;

struct InputState
{
	bool keys[SDL_NUM_SCANCODES];

	InputState();
};

class Car2DMain
{
public:
	Car2DMain();
	~Car2DMain();

	void start();
private:
	SDL_Window* window;
	SDL_GLContext glcontext;
	bool running;
	InputState input_state_current;
	InputState input_state_previous;
	int viewport_width;
	int viewport_height;

	Ticker ticker;

	void setup_context();
	void handle_events();
	void update(float dt);
	void render(float dt, float interpolation);
};
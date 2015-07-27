#pragma once

#include <vector>
#include <map>
#include <SDL2/SDL.h>
#include <yaml-cpp/yaml.h>

struct InputState
{
	bool keys[SDL_NUM_SCANCODES];

	InputState();
};

class Controls
{
public:
	std::vector<SDL_Scancode> accelerate;
	std::vector<SDL_Scancode> reverse;
	std::vector<SDL_Scancode> left;
	std::vector<SDL_Scancode> right;
	std::vector<SDL_Scancode> ebrake;
	std::vector<SDL_Scancode> gear_up;
	std::vector<SDL_Scancode> gear_down;
	std::vector<SDL_Scancode> toggle_automatic;

	Controls(const YAML::Node& config);

	bool is_pressed(const std::vector<SDL_Scancode>& scancodes, const InputState& input_state_current) const;
	bool is_clicked(const std::vector<SDL_Scancode>& scancodes, const InputState& input_state_current, const InputState& input_state_previous) const;
private:
	void load_controls(std::vector<SDL_Scancode>& controls, const YAML::Node& node, const std::map<std::string, SDL_Scancode>& mapping);
};
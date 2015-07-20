#include "input.hpp"
#include <cstring>
#include <map>
#include <iostream>
#include <algorithm>

InputState::InputState()
{
	memset(keys, 0, sizeof(keys));
}

Controls::Controls(const YAML::Node& config)
{
	std::map<std::string, SDL_Scancode> mapping;
	mapping["A"] = SDL_SCANCODE_A;
	mapping["B"] = SDL_SCANCODE_B;
	mapping["C"] = SDL_SCANCODE_C;
	mapping["D"] = SDL_SCANCODE_D;
	mapping["E"] = SDL_SCANCODE_E;
	mapping["F"] = SDL_SCANCODE_F;
	mapping["G"] = SDL_SCANCODE_G;
	mapping["H"] = SDL_SCANCODE_H;
	mapping["I"] = SDL_SCANCODE_I;
	mapping["J"] = SDL_SCANCODE_J;
	mapping["K"] = SDL_SCANCODE_K;
	mapping["L"] = SDL_SCANCODE_L;
	mapping["M"] = SDL_SCANCODE_M;
	mapping["N"] = SDL_SCANCODE_N;
	mapping["O"] = SDL_SCANCODE_O;
	mapping["P"] = SDL_SCANCODE_P;
	mapping["Q"] = SDL_SCANCODE_Q;
	mapping["R"] = SDL_SCANCODE_R;
	mapping["S"] = SDL_SCANCODE_S;
	mapping["T"] = SDL_SCANCODE_T;
	mapping["U"] = SDL_SCANCODE_U;
	mapping["V"] = SDL_SCANCODE_V;
	mapping["W"] = SDL_SCANCODE_W;
	mapping["X"] = SDL_SCANCODE_X;
	mapping["Y"] = SDL_SCANCODE_Y;
	mapping["Z"] = SDL_SCANCODE_Z;
	mapping["LCTRL"] = SDL_SCANCODE_LCTRL;
	mapping["LSHIFT"] = SDL_SCANCODE_LSHIFT;
	mapping["LALT"] = SDL_SCANCODE_LALT;
	mapping["RCTRL"] = SDL_SCANCODE_RCTRL;
	mapping["RSHIFT"] = SDL_SCANCODE_RSHIFT;
	mapping["RALT"] = SDL_SCANCODE_RALT;
	mapping["SPACE"] = SDL_SCANCODE_SPACE;
	mapping["ENTER"] = SDL_SCANCODE_RETURN;
	mapping["RETURN"] = SDL_SCANCODE_RETURN;
	mapping["ESCAPE"] = SDL_SCANCODE_ESCAPE;
	mapping["ESC"] = SDL_SCANCODE_ESCAPE;
	mapping["UP"] = SDL_SCANCODE_UP;
	mapping["DOWN"] = SDL_SCANCODE_DOWN;
	mapping["LEFT"] = SDL_SCANCODE_LEFT;
	mapping["RIGHT"] = SDL_SCANCODE_RIGHT;

	load_controls(accelerate, config["Controls"]["Accelerate"], mapping);
	load_controls(reverse, config["Controls"]["Reverse"], mapping);
	load_controls(left, config["Controls"]["Left"], mapping);
	load_controls(right, config["Controls"]["Right"], mapping);
	load_controls(gear_up, config["Controls"]["GearUp"], mapping);
	load_controls(gear_down, config["Controls"]["GearDown"], mapping);
	load_controls(toggle_automatic, config["Controls"]["ToggleAutomatic"], mapping);
}

bool Controls::is_pressed(const std::vector<SDL_Scancode>& scancodes, const InputState& input_state_current) const
{
	for (int i = 0; i < scancodes.size(); ++i)
	{
		if (input_state_current.keys[scancodes[i]])
			return true;
	}

	return false;
}

bool Controls::is_clicked(const std::vector<SDL_Scancode>& scancodes, const InputState& input_state_current, const InputState& input_state_previous) const
{
	for (int i = 0; i < scancodes.size(); ++i)
	{
		if (input_state_current.keys[scancodes[i]] && !input_state_previous.keys[scancodes[i]])
			return true;
	}

	return false;
}

void Controls::load_controls(std::vector<SDL_Scancode>& controls, const YAML::Node& node, const std::map<std::string, SDL_Scancode>& mapping)
{
	controls.resize(node.size());
	for (int i = 0; i < node.size(); ++i)
	{
		std::string key = node[i].as<std::string>();
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);

		auto it = mapping.find(key);
		if (it == mapping.end())
			std::cerr << "No mapping found for control \"" << node[i].as<std::string>() << "\"" << std::endl;
		else
			controls[i] = it->second;
	}
}
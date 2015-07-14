#pragma once

#include <SDL2/SDL.h>

/*
	Manages the game loop with semi-fixed timestep. Details: http://gafferongames.com/game-physics/fix-your-timestep/.

	Usage:

	Ticker ticker(1.0f / 60.0f, 5);
	ticker.start();
	while (running)
	{
		ticker.tick();
		while (ticker.poll_fixed_tick())
		{
			// Update fixed time step logic. 
		}

		// Render the interpolated scene.
		render(ticker.get_interpolation());
	}
*/
class Ticker
{
public:
	Ticker(float dt, int max_ticks_per_frame);

	void start();
	void tick();
	bool poll_fixed_tick();
	float get_interpolation() const;
	float get_delta_time() const;
	float get_simulated_time() const;
	int get_frame_count() const;
private:
	Uint32 last_clock;
	int frame_count;
	int max_ticks_per_frame;
	int fixed_tick_count;
	float accumulator;
	float dt;
	float t;
	float interpolation;
};
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
	/*
		Initialize the ticker. Specify the delta time (in seconds) for the fixed timestep updates and
		the maximum number of fixed timestep ticks that can happen per frame.
	*/
	Ticker(float dt, int max_ticks_per_frame);

	/*
		Called once before the game loop starts.
	*/
	void start();

	/*
		Called once every frame, updates all values.
	*/
	void tick();

	/*
		Returns true while the fixed timestep logic should be updated. 

		Note: This function changes internal values. Call and update fixed logic until it returns false.
	*/
	bool poll_fixed_tick();

	/*
		Get the interpolation value used for the rendering.
	*/
	float get_interpolation() const;

	/*
		Get the fixed timestep delta time. This is the constant value sent into the constructor.
	*/
	float get_fixed_delta_time() const;

	/*
		Get the amount of time that has been simulated by the fixed timestep logic. This is the number
		of fixed ticks simulated times the delta time.
	*/
	float get_fixed_simulated_time() const;

	/*
		Get the number of fixed timestep logic ticks that have been simulated.
	*/
	int get_fixed_tick_count() const;

	/*
		Get the number of frames simulated. This is equal to the number of times tick() has been called.
	*/
	int get_frame_count() const;

	/*
		Get the time the last frame took. This is equal to the time between the two last tick() calls.
	*/
	float get_frame_time() const;
	
	/*
		Get the number of frames simulated per second. This is 1.0f divided get_frame_time().
	*/
	float get_fps() const;
private:
	Uint32 last_clock;
	Uint32 delta_clock;
	int frame_count;
	int max_ticks_per_frame;
	int current_fixed_tick_count;
	int total_fixed_tick_count;
	float accumulator;
	float dt;
	float t;
	float interpolation;
};
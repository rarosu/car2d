#include "ticker.hpp"

Ticker::Ticker(float dt, int max_ticks_per_frame)
	: last_clock(0)
	, frame_count(0)
	, max_ticks_per_frame(max_ticks_per_frame)
	, fixed_tick_count(0)
	, accumulator(0)
	, dt(dt)
	, t(0)
	, interpolation(0)
{

}

void Ticker::start()
{
	last_clock = SDL_GetTicks();
}

void Ticker::tick()
{
	frame_count++;

	Uint32 current_clock = SDL_GetTicks();
	Uint32 delta_clock = static_cast<Uint32>(current_clock - last_clock);
	last_clock = current_clock;
	float frametime = delta_clock * 0.001f;

	accumulator += frametime;
	fixed_tick_count = accumulator / dt;
	fixed_tick_count = fixed_tick_count <= max_ticks_per_frame ? fixed_tick_count : max_ticks_per_frame;
	t += fixed_tick_count * dt;
	accumulator -= fixed_tick_count * dt;
	interpolation = accumulator / dt - int(accumulator / dt);
}

bool Ticker::poll_fixed_tick()
{
	bool tick = fixed_tick_count >= 1;
	if (tick)
		fixed_tick_count--;
	return tick;
}

float Ticker::get_interpolation() const
{
	return interpolation;
}

float Ticker::get_delta_time() const
{
	return dt;
}

float Ticker::get_simulated_time() const
{
	return t;
}

int Ticker::get_frame_count() const
{
	return frame_count;
}
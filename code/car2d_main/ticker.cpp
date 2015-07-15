#include "ticker.hpp"

Ticker::Ticker(float dt, int max_ticks_per_frame)
	: last_clock(0)
	, delta_clock(0)
	, frame_count(0)
	, max_ticks_per_frame(max_ticks_per_frame)
	, current_fixed_tick_count(0)
	, total_fixed_tick_count(0)
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
	delta_clock = static_cast<Uint32>(current_clock - last_clock);
	last_clock = current_clock;
	float frame_time = delta_clock * 0.001f;

	accumulator += frame_time;
	current_fixed_tick_count = accumulator / dt;
	current_fixed_tick_count = current_fixed_tick_count <= max_ticks_per_frame ? current_fixed_tick_count : max_ticks_per_frame;
	total_fixed_tick_count += current_fixed_tick_count;
	t += current_fixed_tick_count * dt;
	accumulator -= current_fixed_tick_count * dt;
	interpolation = accumulator / dt - int(accumulator / dt);
}

bool Ticker::poll_fixed_tick()
{
	bool tick = current_fixed_tick_count >= 1;
	if (tick)
		current_fixed_tick_count--;
	return tick;
}

float Ticker::get_interpolation() const
{
	return interpolation;
}

float Ticker::get_fixed_delta_time() const
{
	return dt;
}

float Ticker::get_fixed_simulated_time() const
{
	return t;
}

int Ticker::get_fixed_tick_count() const
{
	return total_fixed_tick_count;
}

int Ticker::get_frame_count() const
{
	return frame_count;
}

float Ticker::get_frame_time() const
{
	return delta_clock * 0.001f;
}

float Ticker::get_fps() const
{
	return (delta_clock != 0) ? 1000.0f / delta_clock : 1000.0f;
}
#include "precomp.h"
#include "Core/Application/Config/Config.h"

#include <cmath>

//Declaration of static variable deltatime.
float DM::CLIENT::Config::s_deltaTime;

//Decleration of last time tracked.
std::chrono::time_point<std::chrono::high_resolution_clock> DM::CLIENT::Config::s_lastTime;

//Return the last updated deltatime.
float DM::CLIENT::Config::get_deltaTime()
{
	return s_deltaTime;
}

/// <summary>
/// Update the deltatime to represent the duration between the previous and current frame.
/// Only call this once per start of the frame.
/// </summary>
void DM::CLIENT::Config::update_deltaTime()
{
	const float MAX_DELTATIME = 0.1f;

	const auto current_time = std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - s_lastTime);
	s_lastTime = current_time;

	const float s = static_cast<float>(duration.count() / 1000000.0f);

	s_deltaTime = std::min(s, MAX_DELTATIME);
}

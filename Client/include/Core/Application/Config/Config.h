#pragma once
#include <chrono>

namespace DM 
{
	namespace CLIENT 
	{
		class Config 
		{
        public:
            static float get_deltaTime();
			static void  update_deltaTime();

        private:
			static float s_deltaTime;
			static std::chrono::time_point<std::chrono::high_resolution_clock> s_lastTime;
		};
	}
}
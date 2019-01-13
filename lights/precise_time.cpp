/**
 * precise_time.cpp
 * @author wherewindblow
 * @date   Oct 12, 2018
 */

#include "precise_time.h"

#include <chrono>


namespace lights {

PreciseTime current_precise_time()
{
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t seconds = chrono::system_clock::to_time_t(chrono_time);
	auto duration = chrono_time.time_since_epoch();
	using target_time_type = chrono::nanoseconds;
	auto nanosecond = chrono::duration_cast<target_time_type>(duration).count() % target_time_type::period::den;
	return PreciseTime { seconds, nanosecond };
}

} // namespace lights
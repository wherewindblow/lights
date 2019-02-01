/**
 * precise_time.cpp
 * @author wherewindblow
 * @date   Oct 12, 2018
 */

#include "precise_time.h"

#if LIGHTS_OS != LIGHTS_OS_LINUX
#include <chrono>
#endif


namespace lights {

PreciseTime current_precise_time()
{
#if LIGHTS_OS == LIGHTS_OS_LINUX
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return PreciseTime { ts.tv_sec, ts.tv_nsec };
#else
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t seconds = chrono::system_clock::to_time_t(chrono_time);
	auto duration = chrono_time.time_since_epoch();
	using target_time_type = chrono::nanoseconds;
	auto nanosecond = chrono::duration_cast<target_time_type>(duration).count() % target_time_type::period::den;
	return PreciseTime { seconds, nanosecond };
#endif
}

} // namespace lights
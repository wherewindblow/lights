/**
 * precise_time.h
 * @author wherewindblow
 * @date   Oct 12, 2018
 */

#pragma once

#include <cstdint>
#include <cmath>

#include "format.h"


namespace lights {

/**
 * PreciseTime use to record hight resolution time point.
 */
struct PreciseTime
{
	static const int NANOSECONDS_RATIO = 1000000000;
	static const int MICROSECONDS_RATIO = 1000000;
	static const int MILLISECONDS_RATIO = 1000;

	PreciseTime() :
		PreciseTime(0, 0)
	{}

	PreciseTime(std::int64_t seconds):
		PreciseTime(seconds, 0)
	{}

	PreciseTime(std::int64_t seconds, std::int64_t nanoseconds):
		seconds(seconds), nanoseconds(nanoseconds)
	{}

	std::int64_t seconds;
	std::int64_t nanoseconds;
};


inline bool is_over_flow(std::int64_t a, std::int64_t b)
{
	std::int64_t x = a + b;
	return ((x^a) < 0 && (x^b) < 0);
}


inline PreciseTime operator+(const PreciseTime& left, const PreciseTime& right)
{
	PreciseTime result(left.seconds + right.seconds, left.nanoseconds + right.nanoseconds);
	if (result.nanoseconds >= PreciseTime::NANOSECONDS_RATIO)
	{
		result.nanoseconds -= PreciseTime::NANOSECONDS_RATIO;
		++result.seconds;
	}
	return result;
}


inline PreciseTime operator-(const PreciseTime& left, const PreciseTime& right)
{
	PreciseTime result(left.seconds - right.seconds, left.nanoseconds - right.nanoseconds);
	if (left.nanoseconds < right.nanoseconds)
	{
		--result.seconds;
		result.nanoseconds = std::abs(result.nanoseconds);
	}
	return result;
}


inline bool operator<(const PreciseTime& left, const PreciseTime& right)
{
	if (left.seconds != right.seconds)
	{
		return left.seconds < right.seconds;
	}
	return left.nanoseconds < right.nanoseconds;
}


inline bool operator>(const PreciseTime& left, const PreciseTime& right)
{
	if (left.seconds != right.seconds)
	{
		return left.seconds > right.seconds;
	}
	return left.nanoseconds > right.nanoseconds;
}


/**
 * Returns the current time point.
 */
PreciseTime current_precise_time();


/**
 * Puts precise time to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, const PreciseTime& time)
{
	sink << time.seconds << '.' << time.nanoseconds << 's';
}

} // namespace lights
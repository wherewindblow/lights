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
 * PreciseTime use to record high resolution time point.
 */
struct PreciseTime
{
	static const int NANOSECONDS_OF_SECOND = 1000000000;

	PreciseTime(std::int64_t seconds = 0, std::int64_t nanoseconds = 0):
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


/**
 * Plus operation of PreciseTime.
 */
inline PreciseTime operator+(const PreciseTime& left, const PreciseTime& right)
{
	PreciseTime result(left.seconds + right.seconds, left.nanoseconds + right.nanoseconds);
	if (result.nanoseconds >= PreciseTime::NANOSECONDS_OF_SECOND)
	{
		result.nanoseconds -= PreciseTime::NANOSECONDS_OF_SECOND;
		++result.seconds;
	}
	return result;
}


/**
 * Minus operation of PreciseTime.
 */
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


/**
 * Multiple operation of PreciseTime.
 */
inline PreciseTime operator*(const PreciseTime& time, int n)
{
	PreciseTime result(time.seconds * n, time.nanoseconds * n);
	if (result.nanoseconds >= PreciseTime::NANOSECONDS_OF_SECOND)
	{
		result.seconds += result.nanoseconds / PreciseTime::NANOSECONDS_OF_SECOND;
		result.nanoseconds %= PreciseTime::NANOSECONDS_OF_SECOND;
	}
	return result;
}


/**
 * Divide operation of PreciseTime.
 */
inline PreciseTime operator/(const PreciseTime& time, int n)
{
	PreciseTime result(time.seconds / n, time.nanoseconds / n);
	double sec = time.seconds / static_cast<double>(n);
	double nanosecond = (sec - result.seconds) * PreciseTime::NANOSECONDS_OF_SECOND;
	result.nanoseconds += static_cast<std::int64_t>(nanosecond);
	return result;
}


/**
 * Less than operation of PreciseTime.
 */
inline bool operator<(const PreciseTime& left, const PreciseTime& right)
{
	if (left.seconds != right.seconds)
	{
		return left.seconds < right.seconds;
	}
	return left.nanoseconds < right.nanoseconds;
}


/**
 * Greater than operation of PreciseTime.
 */
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
 * Converts nanosecond to microsecond.
 */
inline std::int64_t nanosecond_to_microsecond(std::int64_t nanosecond)
{
	return nanosecond / 1000;
}

/**
 * Converts microsecond to nanosecond.
 */
inline std::int64_t microsecond_to_nanosecond(std::int64_t microsecond)
{
	return microsecond * 1000;
}

/**
 * Converts nanosecond to millisecond.
 */
inline std::int64_t nanosecond_to_millisecond(std::int64_t nanosecond)
{
	return nanosecond / 1000000;
}

/**
 * Converts millisecond to nanosecond.
 */
inline std::int64_t millisecond_to_nanosecond(std::int64_t millisecond)
{
	return millisecond * 1000000;
}

/**
 * Converts microsecond to millisecond.
 */
inline std::int64_t microsecond_to_millisecond(std::int64_t microsecond)
{
	return microsecond / 1000;
}

/**
 * Converts millisecond to microsecond.
 */
inline std::int64_t millisecond_to_microsecond(std::int64_t millisecond)
{
	return millisecond * 1000;
}

/**
 * Puts precise time to format sink.
 */
template <typename Backend>
void to_string(FormatSink<Backend> sink, const PreciseTime& time)
{
	sink << time.seconds << '.' << pad(time.nanoseconds, '0', 9) << 's';
}

} // namespace lights
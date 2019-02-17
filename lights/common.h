/**
 * common.h
 * @author wherewindblow
 * @date   Apr 14, 2017
 */

#pragma once

#include <cstring>
#include <type_traits>


namespace lights {

/**
 * Uses for internal debug (for format and log).
 */
#define LIGHTS_INTERNAL_DEBUG(format, ...) \
	std::printf(format " [%s:%d][%s]\n", __VA_ARGS__, __FILE__, __LINE__, __func__);

/**
 * Adds static instance function of class.
 */
#define LIGHTS_SINGLETON_INSTANCE(class_name) \
static class_name* instance() \
{ \
	static class_name inst; \
	return &inst; \
}


/**
 * Copies array elements to destination.
 */
template <typename T>
inline void copy_array(T* dest, const T* src, std::size_t num)
{
	std::memcpy(dest, src, sizeof(T) * num);
}

/**
 * Copies array elements to destination with check array size is same.
 */
template <typename T, std::size_t N>
inline void copy_array(T (&dest)[N], const T (&src)[N])
{
	copy_array(dest, src, N);
}


/**
 * Makes all array elements to zero.
 */
template <typename T>
inline void zero_array(T* array, std::size_t num)
{
	std::memset(array, 0, sizeof(T) * num);
}

/**
 * Makes all array elements to zero.
 */
template <typename T, std::size_t N>
inline void zero_array(T (&array)[N])
{
	zero_array(array, N);
}

/**
 * Returns size of array.
 */
template <typename T, std::size_t N>
inline std::size_t size_of_array(T (&array)[N])
{
	return N;
}


/**
 * Checks index is in safe range.
 */
template <typename Index, typename Num, typename Max>
inline bool is_safe_index(Index index, Num num, Max max)
{
	using UnsignedIndex = std::make_unsigned_t<Index>;
	using UnsignedNum = std::make_unsigned_t<Num>;
	using UnsignedMax = std::make_unsigned_t<Max>;
	return index >= 0 &&
		static_cast<UnsignedIndex>(index) < static_cast<UnsignedNum>(num) &&
		static_cast<UnsignedIndex>(index) < static_cast<UnsignedMax>(max);
}

/**
 * Checks index is in safe range.
 */
template <typename Index, typename Num, typename T, std::size_t N>
inline bool is_safe_index(Index index, Num num, const T (&array)[N])
{
	return is_safe_index(index, num, N);
}

/**
 * Checks index is in safe range.
 */
template <typename Index, typename Num>
inline bool is_safe_index(Index index, Num num)
{
	return is_safe_index(index, num, num);
}

/**
 * Checks index is in safe range.
 */
template <typename Index, typename T, std::size_t N>
inline bool is_safe_index(Index index, const T (&array)[N])
{
	return is_safe_index(index, N);
}


/**
 * Checks have flag in all set.
 */
template <typename T>
inline bool have_flag(T all_set, T flag)
{
	return all_set & flag;
}

/**
 * Sets flag in all set.
 */
template <typename T>
inline void set_flag(T& all_set, T flag)
{
	all_set |= flag;
}

/**
 * Clears flag in all set.
 */
template <typename T>
inline void clear_flags(T& all_set, T flag)
{
	all_set &= (~flag);
}

} // namespace lights

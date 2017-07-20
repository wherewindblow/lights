/**
 * common.h
 * @author wherewindblow
 * @date   Apr 14, 2017
 */

#pragma once

#include <cstring>
#include <algorithm>


namespace lights {

template <typename T>
inline void copy_array(T* dest, const T* src, std::size_t num)
{
	std::memcpy(dest, src, sizeof(T) * num);
}

template <typename T, std::size_t N>
inline void copy_array(T (&dest)[N], const T (&src)[N])
{
	copy_array(dest, src, N);
}


template <typename T>
inline void zero_array(T* array, std::size_t num)
{
	std::memset(array, 0, sizeof(T) * num);
}

template <typename T, std::size_t N>
inline void zero_array(T (&array)[N])
{
	zero_array(array, N);
}


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

template <typename Index, typename Num, typename T, std::size_t N>
inline bool is_safe_index(Index index, Num num, const T (&array)[N])
{
	return is_safe_index(index, num, N);
}

template <typename Index, typename Num>
inline bool is_safe_index(Index index, Num num)
{
	return is_safe_index(index, num, num);
}

template <typename Index, typename T, std::size_t N>
inline bool is_safe_index(Index index, const T (&array)[N])
{
	return is_safe_index(index, N);
}


template <typename T>
inline bool have_flag(T all_set, T flag)
{
	return all_set & flag;
}

template <typename T>
inline void set_flag(T& all_set, T flag)
{
	all_set |= flag;
}

template <typename T>
inline void clear_flags(T& all_set, T flag)
{
	all_set &= (~flag);
}

} // namespace lights

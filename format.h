/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstddef>
#include <string>


namespace lights {
namespace details {

template <typename T>
inline void append(std::string& sink, T arg)
{
	sink.append(std::to_string(arg));
}

inline void append(std::string& sink, char* arg)
{
	sink.append(arg);
}

inline void append(std::string& sink, const char* arg)
{
	sink.append(arg);
}

inline void append(std::string& sink, const std::string& arg)
{
	sink.append(arg);
}


inline void format_impl(std::string& result, const char* fmt)
{
	result.append(fmt);
}

template <typename Arg, typename... Args>
void format_impl(std::string& result, const char* fmt, Arg value, Args... args)
{
	std::size_t i = 0;
	for (; fmt[i] != '\0'; ++i)
	{
		if (fmt[i] == '{' &&
			fmt[i+1] != '\0' &&
			fmt[i+1] == '}')
		{
			break;
		}
	}

	result.append(fmt, i);
	if (fmt[i] != '\0')
	{
		append(result, value);
		format_impl(result, fmt + i + 2, args...);
	}
}

} // namespace details


inline std::string format(const char* fmt)
{
	return std::string(fmt);
}

template <typename Arg, typename... Args>
inline std::string format(const char* fmt, Arg value, Args... args)
{
	std::string result;
	details::format_impl(result, fmt, value, args...);
	return result;
}

} // namespace lights
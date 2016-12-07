/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstring>
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
	std::size_t len = std::strlen(fmt);
	for (std::size_t i = 0; i < len; ++i)
	{
		if (fmt[i] == '{' &&
			i + 1 < len &&
			fmt[i+1] == '}')
		{
			append(result, value);
			format_impl(result, fmt + i + 2, args...);
			break;
		}
		else
		{
			result.push_back(fmt[i]);
		}
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
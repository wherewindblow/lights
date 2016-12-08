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

inline void to_string(std::string& sink, bool is)
{
	sink.append(is ? "true" : "false");
}

inline void to_string(std::string& sink, char ch)
{
	sink.push_back(ch);
}

void to_string(std::string& sink, short n);
void to_string(std::string& sink, unsigned short n);
void to_string(std::string& sink, int n);
void to_string(std::string& sink, unsigned int n);
void to_string(std::string& sink, long n);
void to_string(std::string& sink, unsigned long n);
void to_string(std::string& sink, long long n);
void to_string(std::string& sink, unsigned long long n);
void to_string(std::string& sink, float n);
void to_string(std::string& sink, double n);
void to_string(std::string& sink, long double n);


template <typename T>
inline void append(std::string& sink, T arg)
{
	to_string(sink, arg);
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
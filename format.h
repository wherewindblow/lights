/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstddef>
#include <string>
#include <limits>


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

inline void to_string(std::string& sink, short n)
{
	char buf[std::numeric_limits<short>::digits10 + 1];
	int writen = std::sprintf(buf, "%hd", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, unsigned short n)
{
	char buf[std::numeric_limits<unsigned short>::digits10 + 1];
	int writen = std::sprintf(buf, "%hu", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, int n)
{
	char buf[std::numeric_limits<int>::digits10 + 1];
	int writen = std::sprintf(buf, "%d", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, unsigned int n)
{
	char buf[std::numeric_limits<unsigned int>::digits10 + 1];
	int writen = std::sprintf(buf, "%u", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, long n)
{
	char buf[std::numeric_limits<long>::digits10 + 1];
	int writen = std::sprintf(buf, "%ld", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, unsigned long n)
{
	char buf[std::numeric_limits<unsigned long>::digits10 + 1];
	int writen = std::sprintf(buf, "%lu", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, long long n)
{
	char buf[std::numeric_limits<long long>::digits10 + 1];
	int writen = std::sprintf(buf, "%lld", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, unsigned long long n)
{
	char buf[std::numeric_limits<unsigned long long>::digits10 + 1];
	int writen = std::sprintf(buf, "%llu", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, float n)
{
	char buf[std::numeric_limits<float>::max_exponent10 + 1 +
		std::numeric_limits<float>::digits10 + 1];
	int writen = std::sprintf(buf, "%f", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, double n)
{
	// 100 is the max exponent10 of double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%f", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}

inline void to_string(std::string& sink, long double n)
{
	// 100 is the max exponent10 of long double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<long double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%Lf", n);
	sink.append(buf, static_cast<std::size_t>(writen));
}


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
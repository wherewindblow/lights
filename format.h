/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstddef>
#include <string>
#include <limits>
#include <sstream>


namespace lights {
namespace details {

/**
 * StringBuffer that reference on a std::string.
 * Instead of std::stringbuf to optimize performance.
 */
class StringBuffer: public std::streambuf
{
public:
	StringBuffer(std::string& sink) :
		m_sink(sink) {}

	virtual int_type overflow(int_type ch) override
	{
		m_sink.push_back(static_cast<char>(ch));
		return ch;
	}

	virtual std::streamsize	xsputn(const char* s, std::streamsize n) override
	{
		m_sink.append(s, static_cast<std::size_t>(n));
		return n;
	}

private:
	std::string& m_sink;
};


class IntegerFormater
{
public:
	IntegerFormater() :
		m_begin(m_buf + sizeof(m_buf) - 1)
	{
		m_buf[sizeof(m_buf) - 1] = '\0';
	}

	const char* format_unsigned(std::uintmax_t n)
	{
		while (n != 0)
		{
			--m_begin;
			*m_begin = '0' + static_cast<char>(n % 10);
			n /= 10;
		}

		return m_begin;
	}

	const char* format_signed(std::intmax_t n)
	{
		std::uintmax_t absolute = static_cast<std::uintmax_t>(n);
		bool negative = n < 0;
		if (negative)
		{
			absolute = 0 - absolute;
		}
		this->format_unsigned(absolute);
		if (negative)
		{
			--m_begin;
			*m_begin = '-';
		}
		return m_begin;
	}

	const char* format(unsigned short n)
	{
		return this->format_unsigned(n);
	}

	const char* format(unsigned int n)
	{
		return this->format_unsigned(n);
	}

	const char* format(unsigned long n)
	{
		return this->format_unsigned(n);
	}

	const char* format(unsigned long long n)
	{
		return this->format_unsigned(n);
	}

	const char* format(short n)
	{
		return this->format_signed(n);
	}

	const char* format(int n)
	{
		return this->format_signed(n);
	}

	const char* format(long n)
	{
		return this->format_signed(n);
	}

	const char* format(long long n)
	{
		return this->format_signed(n);
	}

private:
	char m_buf[std::numeric_limits<std::uintmax_t>::digits10 + 1 + 1];
	char* m_begin;
};


} // namespace details

/**
 * To convert @c value to string in the end of @ sink
 * that use insertion operator with std::ostream and T.
 * Aim to support format with
 *   `std::ostream operator<< (std::ostream& out, const T& value)`
 * @param sink   string reference.
 * @param value  User type.
 */
template <typename T>
inline void to_string(std::string& sink, const T& value)
{
	details::StringBuffer buf(sink);
	std::ostream ostream(&buf);
	ostream << value;
}

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
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, unsigned short n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, int n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, unsigned int n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, long n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, unsigned long n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, long long n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
}

inline void to_string(std::string& sink, unsigned long long n)
{
	details::IntegerFormater formater;
	sink.append(formater.format(n));
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


/**
 * Append the @c arg to the end of sink. It'll invoke @c to_string
 * Aim to support append not char* and not std::string
 * to the end of sink.
 * @param sink  string.
 * @param arg   Any type.
 */
template <typename T>
inline void append(std::string& sink, const T& arg)
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

/**
 * Insert value into the sink. It'll invoke @c append
 * Aim to support user can define
 *   `std::string& operator<< (std::string& sink, T value)`
 * to format with user type
 * @param sink   string.
 * @param value  Build-in type or user type.
 * @return The reference of sink.
 */
template <typename T>
inline std::string& operator<< (std::string& sink, const T& value)
{
	append(sink, value);
	return sink;
}


inline void write(std::string& result, const char* fmt)
{
	result.append(fmt);
}

template <typename Arg, typename ... Args>
void write(std::string& result, const char* fmt, Arg value, const Args& ... args)
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
		result << value;
		write(result, fmt + i + 2, args ...);
	}
}


inline std::string format(const char* fmt)
{
	return std::string(fmt);
}

/**
 * Format string that use @c fmt and @c args ...
 * @param fmt   Format that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @note If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `std::string& operator<< (std::string& sink, const T& value);`
 *         3) `void append(std::string& sink, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2) and 3) Optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3) and 1).
 */
template <typename... Args>
inline std::string format(const char* fmt, const Args& ... args)
{
	std::string result;
	write(result, fmt, args ...);
	return result;
}

} // namespace lights
/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstddef>
#include <cerrno>
#include <string>
#include <limits>
#include <sstream>

#include <errno.h>

#include "config.h"


namespace lights {

/**
 * A wrapper aim to identify this is a error not an integer.
 */
struct ErrorNumber
{
	int number;
};

inline ErrorNumber to_error(int error_no)
{
	return ErrorNumber{ error_no };
}

inline ErrorNumber current_error()
{
	return to_error(errno);
}


/**
 * View of string, can reduce data copy.
 */
struct StringView
{
	const char* string;
	std::size_t length;
};


/**
 * StringAdapter adapter Sink to a abstract string.
 * It's light weight and can be use as pass by value.
 * @tparam Sink  To be write backend.
 * @note This class template cannot to be use, and must to be
 *       instantiation and implement the method by user.
 */
template <typename Sink>
class StringAdapter
{
public:
	StringAdapter(Sink& sink) = delete;

	void append(char ch)
	{
		m_sink.append(ch);
	};

	void append(const char* str, std::size_t len)
	{
		while (len != 0)
		{
			this->append(*str);
			++str;
			--len;
		}
	}

	void append(const char* str)
	{
		this->append(str, std::strlen(str));
	}

private:
	Sink& m_sink;
};

/**
 * Helper function to create StringAdapter.
 */
template <typename T>
StringAdapter<T> make_string_adapter(T& value)
{
	return StringAdapter<T>(value);
}


template <>
class StringAdapter<std::string>
{
public:
	StringAdapter(std::string& sink) : m_sink(sink) {}

	void append(char ch)
	{
		m_sink.push_back(ch);
	}

	void append(const char* str, std::size_t len)
	{
		m_sink.append(str, len);
	}

	void append(const char* str)
	{
		m_sink.append(str);
	}

private:
	std::string& m_sink;
};


namespace details {

/**
 * StringBuffer that reference on a string.
 * Instead of std::stringbuf to optimize performance.
 */
template <typename Sink>
class StringBuffer: public std::streambuf
{
public:
	StringBuffer(Sink& sink) :
		m_string(sink) {}

	virtual int_type overflow(int_type ch) override
	{
		m_string.append(static_cast<char>(ch));
		return ch;
	}

	virtual std::streamsize	xsputn(const char* s, std::streamsize n) override
	{
		m_string.append(s, static_cast<std::size_t>(n));
		return n;
	}

private:
	StringAdapter<Sink>& m_string;
};


#ifdef LIGHTS_FORMAT_INTEGER_OPTIMIZE
static constexpr char digists[] =
	"0001020304050607080910111213141516171819"
	"2021222324252627282930313233343536373839"
	"4041424344454647484950515253545556575859"
	"6061626364656667686970717273747576777879"
	"8081828384858687888990919293949596979899";
#endif

class IntegerFormater
{
public:
	IntegerFormater()
	{
		m_buf[sizeof(m_buf) - 1] = '\0';
	}

#define LIGHTS_INTEGER_FORMATER_FORMAT_UNSIGNED(Type) \
	const char* format(Type n)           \
	{                                    \
		this->reset_state();             \
		return this->format_unsigned(n); \
	}

	LIGHTS_INTEGER_FORMATER_FORMAT_UNSIGNED(unsigned short)
	LIGHTS_INTEGER_FORMATER_FORMAT_UNSIGNED(unsigned int)
	LIGHTS_INTEGER_FORMATER_FORMAT_UNSIGNED(unsigned long)
	LIGHTS_INTEGER_FORMATER_FORMAT_UNSIGNED(unsigned long long)

#undef LIGHTS_INTEGER_FORMATER_FORMAT_UNSIGNED

#define LIGHTS_INTEGER_FORMATER_FORMAT_SIGNED(Type) \
	const char* format(Type n)           \
	{                                    \
		this->reset_state();             \
		return this->format_signed(n); \
	}

	LIGHTS_INTEGER_FORMATER_FORMAT_SIGNED(short)
	LIGHTS_INTEGER_FORMATER_FORMAT_SIGNED(int)
	LIGHTS_INTEGER_FORMATER_FORMAT_SIGNED(long)
	LIGHTS_INTEGER_FORMATER_FORMAT_SIGNED(long long)

#undef LIGHTS_INTEGER_FORMATER_FORMAT_SIGNED

private:
	const char* format_unsigned(std::uintmax_t n)
	{
		if (n == 0)
		{
			--m_begin;
			*m_begin = '0';
		}
		else
		{
#ifndef LIGHTS_FORMAT_INTEGER_OPTIMIZE
			while (n != 0)
			{
				--m_begin;
				*m_begin = '0' + static_cast<char>(n % 10);
				n /= 10;
			}
#else
			while (n >= 100)
			{
				auto index = n % 100 * 2;
				--m_begin;
				*m_begin = digists[index + 1];
				--m_begin;
				*m_begin = digists[index];
				n /= 100;
			}

			if (n < 10) // Single digit.
			{
				--m_begin;
				*m_begin = '0' + static_cast<char>(n);
			}
			else // Double digits.
			{
				auto index = n * 2;
				--m_begin;
				*m_begin = digists[index + 1];
				--m_begin;
				*m_begin = digists[index];
			}
#endif
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

	void reset_state()
	{
		m_begin = m_buf + sizeof(m_buf) - 1;
	}

	char m_buf[std::numeric_limits<std::uintmax_t>::digits10 + 1 + 1];
	char* m_begin;
};


class ErrorFormater
{
public:
	const char* format(int errer_no)
	{
		// Not use sys_nerr and sys_errlist directly, although the are easy to control.
		// Because sys_nerr may bigger that sys_errlist size and sys_errlist may have't
		// all string for errno. In the way will lead to segment fault.
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE  // posix verion
		if (strerror_r(errer_no, m_buf, sizeof(m_buf)) == 0)
		{
			return m_buf;
		}
		else
		{
			return "Unkown error";
		}
#else // gnu version, m_buf is not use when is known error, but it's use when is unkown
		return strerror_r(errer_no, m_buf, sizeof(m_buf));
#endif
	}

	const char* format(ErrorNumber errer_no)
	{
		return this->format(errer_no.number);
	}

	const char* format_current_error()
	{
		return this->format(errno);
	}

private:
	// 100 charater is enough to put all error string
	// on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2) (Englist Version).
	// In another languague version may have to change to largger to
	// hold all message.
	char m_buf[100];
};

} // namespace details

/**
 * To convert @c value to string in the end of @ sink
 * that use insertion operator with std::ostream and T.
 * Aim to support format with
 *   `std::ostream& operator<< (std::ostream& out, const T& value)`
 * @param out    Abstract string.
 * @param value  User type.
 */
template <typename Sink, typename T>
inline void to_string(StringAdapter<Sink> out, const T& value)
{
	details::StringBuffer<Sink> buf(out);
	std::ostream ostream(&buf);
	ostream << value;
}

template <typename Sink>
inline void to_string(StringAdapter<Sink> out, bool is)
{
	out.append(is ? "true" : "false");
}

template <typename Sink>
inline void to_string(StringAdapter<Sink> out, char ch)
{
	out.append(ch);
}


#define LIGHTS_INTEGER_TO_STRING(Type)            \
template <typename Sink>                          \
inline void to_string(StringAdapter<Sink> out, Type n) \
{                                                 \
	details::IntegerFormater formater;            \
	out.append(formater.format(n));               \
}

LIGHTS_INTEGER_TO_STRING(short)
LIGHTS_INTEGER_TO_STRING(int)
LIGHTS_INTEGER_TO_STRING(long)
LIGHTS_INTEGER_TO_STRING(long long)
LIGHTS_INTEGER_TO_STRING(unsigned short)
LIGHTS_INTEGER_TO_STRING(unsigned int)
LIGHTS_INTEGER_TO_STRING(unsigned long)
LIGHTS_INTEGER_TO_STRING(unsigned long long)

#undef LIGHTS_INTEGER_TO_STRING


template <typename Sink>
inline void to_string(StringAdapter<Sink> out, float n)
{
	char buf[std::numeric_limits<float>::max_exponent10 + 1 +
		std::numeric_limits<float>::digits10 + 1];
	int writen = std::sprintf(buf, "%f", n);
	out.append(buf, static_cast<std::size_t>(writen));
}

template <typename Sink>
inline void to_string(StringAdapter<Sink> out, double n)
{
	// 100 is the max exponent10 of double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%f", n);
	out.append(buf, static_cast<std::size_t>(writen));
}

template <typename Sink>
inline void to_string(StringAdapter<Sink> out, long double n)
{
	// 100 is the max exponent10 of long double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<long double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%Lf", n);
	out.append(buf, static_cast<std::size_t>(writen));
}

template <typename Sink>
inline void to_string(StringAdapter<Sink> out, ErrorNumber error_no)
{
	details::ErrorFormater formater;
	out.append(formater.format(error_no));
}

template <typename Sink>
inline void to_string(StringAdapter<Sink> out, const StringView& value)
{
	out.append(value.string, value.length);
}


/**
 * Append the @c arg to the end of @c out. It'll invoke @c to_string
 * Aim to support append not char* and not std::string
 * to the end of @c out.
 * @param out    Abstract string.
 * @param value  Any type.
 */
template <typename Sink, typename T>
inline void append(StringAdapter<Sink> out, const T& value)
{
	to_string(out, value);
}

template <typename Sink>
inline void append(StringAdapter<Sink> out, char* str)
{
	out.append(str);
}

template <typename Sink>
inline void append(StringAdapter<Sink> out, const char* str)
{
	out.append(str);
}

template <typename Sink>
inline void append(StringAdapter<Sink> out, const std::string& str)
{
	out.append(str.c_str(), str.length());
}

/**
 * Insert value into the sink. It'll invoke @c append
 * Aim to support user can define
 *   `StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value)`
 * to format with user type
 * @param out    Abstract string.
 * @param value  Built-in type or user type.
 * @return StringAdapter.
 */
template <typename Sink, typename T>
inline StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value)
{
	append(out, value);
	return out;
}


template <typename Sink>
inline void write(StringAdapter<Sink> out, const char* fmt)
{
	out.append(fmt);
}

/**
 * Write to the end of string that use @c fmt and @c args ...
 * @param sink  Output holder.
 * @param fmt   Format that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @note If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value);`
 *         3) `void append(StringAdapter<Sink> out, const T& value);`
 *         4) `void to_string(StringAdapter<Sink> out, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2), 3) and 4) is optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3), 4) and 1).
 */
template <typename Sink, typename Arg, typename ... Args>
void write(StringAdapter<Sink> out, const char* fmt, const Arg& value, const Args& ... args)
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

	out.append(fmt, i);
	if (fmt[i] != '\0')
	{
		append(out, value);
		write(out, fmt + i + 2, args ...);
	}
}

/**
 * Write to the end of string that use @c fmt and @c args ...
 * @param out  Output holder.
 * @param fmt   Format that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @note If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value);`
 *         3) `void append(StringAdapter<Sink> out, const T& value);`
 *         4) `void to_string(StringAdapter<Sink> out, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2), 3) and 4) is optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3), 4) and 1).
 */
template <typename Sink, typename Arg, typename ... Args>
inline void write(StringAdapter<Sink> out, const std::string& fmt, const Arg& value, const Args& ... args)
{
	write(out, fmt.c_str(), value, args ...);
}


/**
 * Format string that use @c fmt and @c args ...
 * @param fmt   Format that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @note If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value);`
 *         3) `void append(StringAdapter<Sink> out, const T& value);`
 *         4) `void to_string(StringAdapter<Sink> out, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2), 3) and 4) is optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3), 4) and 1).
 */
template <typename... Args>
inline std::string format(const char* fmt, const Args& ... args)
{
	std::string out;
	write(make_string_adapter(out), fmt, args ...);
	return out;
}

/**
 * Format string that use @c fmt and @c args ...
 * @param fmt   Format that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @note If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value);`
 *         3) `void append(StringAdapter<Sink> out, const T& value);`
 *         4) `void to_string(StringAdapter<Sink> out, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2), 3) and 4) is optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3), 4) and 1).
 */
template <typename... Args>
inline std::string format(const std::string& fmt, const Args& ... args)
{
	return format(fmt.c_str(), args ...);
}

} // namespace lights
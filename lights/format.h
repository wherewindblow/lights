/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <limits>
#include <functional>

#include "config.h"
#include "env.h"
#include "sequence.h"
#include "common.h"


namespace lights {

#define LIGHTS_IMPLEMENT_SIGNED_INTEGER_FUNCTION(macro) \
	macro(std::int8_t)        \
	macro(std::int16_t)       \
	macro(std::int32_t)       \
	macro(std::int64_t)

#define LIGHTS_IMPLEMENT_UNSIGNED_INTEGER_FUNCTION(macro) \
	macro(std::uint8_t)       \
	macro(std::uint16_t)      \
	macro(std::uint32_t)      \
	macro(std::uint64_t)

#define LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(macro) \
	LIGHTS_IMPLEMENT_SIGNED_INTEGER_FUNCTION(macro) \
	LIGHTS_IMPLEMENT_UNSIGNED_INTEGER_FUNCTION(macro)


/**
 * To set this type to signed char for convenient to assign to
 * any type. Char is the smallest basic type and assign to other
 * type will not lose data. And signed will ensure this value is -1
 * not 255, -1 signed char fit into int will be -1, fit into signed
 * int will be the biggest value of signed int. If unsigned fit
 * into int will be 255 and unsigned int also is this value.
 */
static const signed char INVALID_INDEX = -1;


template <typename Value, typename Tag>
struct IntegerFormatSpec
{
	Value value;
	Tag tag;
	int width = INVALID_INDEX;
	char fill;
};


/**
 * A wrapper aim to identity this is a error not an integer.
 */
struct ErrorNumber
{
	ErrorNumber() = default;
	explicit ErrorNumber(int no): value(no) {}

	int value;
};

inline ErrorNumber current_error()
{
	return ErrorNumber(errno);
}

/**
 * A wrapper aim to identity this is a time not an integer.
 */
struct Timestamp
{
	Timestamp() = default;
	explicit Timestamp(std::time_t time): value(time) {}

	std::time_t value;
};

inline Timestamp current_timestamp()
{
	return Timestamp(std::time(nullptr));
}


/**
 * Adapter Sink to provide same interface.
 * It's light weight and can be use as pass by value.
 * @tparam Sink  To be write backend.
 * @note This class template cannot to be use, and must to be
 *       explicit specialization and implement the method by user.
 */
template <typename Sink>
class FormatSinkAdapter
{
public:
	explicit FormatSinkAdapter(Sink& sink) = delete;

	void append(char ch)
	{
		m_sink.append(ch);
	};

	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	void append(StringView str)
	{
		while (str.length() != 0)
		{
			this->append(*str.data());
			str.move_forward(1);
		}
	}

private:
	Sink& m_sink;
};

/**
 * Helper function to create adapter.
 */
template <typename T>
inline FormatSinkAdapter<T> make_format_sink_adapter(T& value)
{
	return FormatSinkAdapter<T>(value);
}

/**
 * Explicit tempate specialization of std::string
 */
template <>
class FormatSinkAdapter<std::string>
{
public:
	explicit FormatSinkAdapter(std::string& sink) : m_sink(sink) {}

	void append(char ch)
	{
		m_sink.push_back(ch);
	}

	void append(std::size_t num, char ch)
	{
		m_sink.append(num, ch);
	}

	void append(StringView str)
	{
		m_sink.append(str.data(), str.length());
	}

private:
	std::string& m_sink;
};


namespace details {

#ifdef LIGHTS_DETAILS_INTEGER_FORMATER_OPTIMIZE
static constexpr char digists[] =
	"0001020304050607080910111213141516171819"
	"2021222324252627282930313233343536373839"
	"4041424344454647484950515253545556575859"
	"6061626364656667686970717273747576777879"
	"8081828384858687888990919293949596979899";
#endif


/**
 * Get the need space of format integer @c n.
 * @tparam Integer  Any integer type.
 * @param n         A integer that type of Integer.
 */
template <typename Integer>
std::size_t format_need_space(Integer n);

/**
 * Formats a integer @c n to @c output.
 * @tparam Integer  Any integer type.
 * @param n         A integer that type of Integer.
 * @param output    Point to that last place of ouput.
 * @return  The result that point to first digit.
 * @note Formats character backwards to @c output and @c *output this pos is not use.
 */
template <typename Integer>
char* format_integer(Integer n, char* output);


class IntegerFormater
{
public:
	IntegerFormater()
	{
		m_buf[sizeof(m_buf) - 1] = '\0';
	}

	template <typename Integer>
	StringView format(Integer num)
	{
		reset_state();
		return format_integer(num, m_begin);
	}

private:
	void reset_state()
	{
		m_begin = m_buf + sizeof(m_buf) - 1;
	}

	char m_buf[std::numeric_limits<std::uintmax_t>::digits10 + 1 + 1];
	char* m_begin;
};


template <typename Sink>
inline FormatSinkAdapter<Sink> write_2_digit(FormatSinkAdapter<Sink> out, unsigned num)
{
	if (num >= 10)
	{
		out << num;
	}
	else
	{
		out << '0' << num;
	}
	return out;
}


/**
 * Integer format spec tag is only use to identity which spec is indicate.
 */
struct BinarySpecTag {};
struct OctalSpecTag {};
struct DecimalSpecTag {};
struct HexSpecLowerCaseTag {};
struct HexSpecUpperCaseTag {};


/**
 * Converts integer to binary character.
 * @param value  Integer
 * @return Binary character
 */
template <typename T>
inline char to_binary_char(T value)
{
	return value ? '1' : '0';
}

/**
 * Converts integer to lower case hex character.
 * @param ch  Integer that on range [0, 16)
 * @return Hex character
 */
inline char to_hex_lower_case_char(char ch)
{
	if (ch < 10)
	{
		return '0' + ch;
	}
	else
	{
		return 'a' + ch - static_cast<char>(10);
	}
}

/**
 * Converts integer to upper case hex character.
 * @param ch  Integer that on range [0, 16)
 * @return Hex character
 */
inline char to_hex_upper_case_char(char ch)
{
	if (ch < 10)
	{
		return '0' + ch;
	}
	else
	{
		return 'A' + ch - static_cast<char>(10);
	}
}


template <typename Tag>
struct HexConvertHandler;

template <>
struct HexConvertHandler<HexSpecLowerCaseTag>
{
	using Handler = char(*)(char);
	static constexpr Handler handler = to_hex_lower_case_char;
};

template <>
struct HexConvertHandler<HexSpecUpperCaseTag>
{
	using Handler = char(*)(char);
	static constexpr Handler handler = to_hex_upper_case_char;
};



template <typename Integer, bool is_signed = std::is_signed<Integer>::value>
class BinaryFormater;

/**
 * BinaryFormater with unsigned integer.
 */
template <typename UnsignedInteger>
class BinaryFormater<UnsignedInteger, false>
{
public:
	template <typename Sink>
	static void format(FormatSinkAdapter<Sink> out,
					   IntegerFormatSpec<UnsignedInteger, details::BinarySpecTag> spec,
					   bool negative = false);
};

template <typename UnsignedInteger>
template <typename Sink>
void BinaryFormater<UnsignedInteger, false>::format(FormatSinkAdapter<Sink> out,
													IntegerFormatSpec<UnsignedInteger, details::BinarySpecTag> spec,
													bool negative)
{
	UnsignedInteger absolute_value = spec.value;
	int num = 0;
	do
	{
		++num;
	} while (absolute_value >>= 1);

	int width = negative ? num + 1 : num;
	if (spec.width != INVALID_INDEX && width < spec.width)
	{
		out.append(spec.width - width, spec.fill);
	}

	if (negative)
	{
		out.append('-');
	}

	UnsignedInteger mask = 1ul << (num - 1);
	while (mask != 0)
	{
		char ch = to_binary_char(spec.value & mask);
		out.append(ch);
		mask >>= 1;
	}
}


template <typename Integer, bool is_signed = std::is_signed<Integer>::value>
class OctalFormater;

/**
 * OctalFormater with unsigned integer.
 */
template <typename UnsignedInteger>
class OctalFormater<UnsignedInteger, false>
{
public:
	template <typename Sink>
	static void format(FormatSinkAdapter<Sink> out,
					   IntegerFormatSpec<UnsignedInteger, details::OctalSpecTag> spec,
					   bool negative = false);
};

template <typename UnsignedInteger>
template <typename Sink>
void OctalFormater<UnsignedInteger, false>::format(FormatSinkAdapter<Sink> out,
												   IntegerFormatSpec<UnsignedInteger, details::OctalSpecTag> spec,
												   bool negative)
{
	const int digit_of_spec = 3;
	int len = std::numeric_limits<UnsignedInteger>::digits / digit_of_spec;
	const int remain = std::numeric_limits<UnsignedInteger>::digits % digit_of_spec;
	if (remain != 0)
	{
		++len;
	}

	char str[len];
	UnsignedInteger absolute_value = spec.value;
	char* ptr = &str[len - 1];
	do
	{
		*ptr = static_cast<char>('0' + (absolute_value & 7)); // 7 binary: 0000, 0111
		--ptr;
	} while ((absolute_value >>= digit_of_spec) != 0);
	++ptr;

	std::size_t num = str + len - ptr;
	int width = static_cast<int>(negative ? num + 1 : num);
	if (spec.width != INVALID_INDEX && width < spec.width)
	{
		out.append(spec.width - width, spec.fill);
	}

	if (negative)
	{
		out.append('-');
	}
	out.append({ptr, num});
}


template <typename Integer, bool is_signed = std::is_signed<Integer>::value>
class HexFormater;

/**
 * HexFormater with unsigned integer.
 */
template <typename UnsignedInteger>
class HexFormater<UnsignedInteger, false>
{
public:
	template <typename Sink, typename Tag>
	static void format(FormatSinkAdapter<Sink> out,
					   IntegerFormatSpec<UnsignedInteger, Tag> spec,
					   bool negative = false);
};

template <typename UnsignedInteger>
template <typename Sink, typename Tag>
void HexFormater<UnsignedInteger, false>::format(FormatSinkAdapter<Sink> out,
												 IntegerFormatSpec<UnsignedInteger, Tag> spec,
												 bool negative)
{
	const int digit_of_spec = 4;
	int len = std::numeric_limits<UnsignedInteger>::digits / digit_of_spec;
	const int remain = std::numeric_limits<UnsignedInteger>::digits % digit_of_spec;
	if (remain != 0)
	{
		++len;
	}

	char str[len];
	UnsignedInteger absolute_value = spec.value;
	char* ptr = &str[len - 1];
	do
	{
		char ch = static_cast<char>(absolute_value & 15); // 15 binary: 0000, 1111
		*ptr = HexConvertHandler<Tag>::handler(ch);
		--ptr;
	} while ((absolute_value >>= digit_of_spec) != 0);
	++ptr;

	std::size_t num = str + len - ptr;
	int width = static_cast<int>(negative ? num + 1 : num);
	if (spec.width != INVALID_INDEX && width < spec.width)
	{
		out.append(spec.width - width, spec.fill);
	}

	if (negative)
	{
		out.append('-');
	}
	out.append({ptr, num});
}


/**
 * Formater with signed integer.
 */
#define LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(formater_name)                          \
template <typename SignedInteger>                                                     \
class formater_name<SignedInteger, true>                                              \
{                                                                                     \
public:                                                                               \
	template <typename Sink, typename Tag>                                            \
	static void format(FormatSinkAdapter<Sink> out, IntegerFormatSpec<SignedInteger, Tag> spec) \
	{                                                                                 \
		using UnsignedInteger = std::make_unsigned_t<SignedInteger>;                  \
		UnsignedInteger absolute = static_cast<UnsignedInteger>(spec.value);          \
                                                                                      \
		bool negative = spec.value < 0;                                               \
		if (negative)                                                                 \
		{                                                                             \
			absolute = 0 - absolute;                                                  \
		}                                                                             \
                                                                                      \
		IntegerFormatSpec<UnsignedInteger, Tag> new_spec = { absolute, Tag(), spec.width, spec.fill }; \
		formater_name<UnsignedInteger>::format(out, new_spec, negative);              \
	}                                                                                 \
};

LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(BinaryFormater)
LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(OctalFormater)
LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(HexFormater)

#undef LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER

} // namespace details


template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, bool is)
{
	out.append(is ? "true" : "false");
}

template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, char ch)
{
	out.append(ch);
}

/**
 * Formats all type of integer to string.
 * @details Why must explicit specialization it? Because if not do that, SFINAE will
 *          pass user-defined type into this template function and cause compile error.
 */
#define LIGHTS_INTEGER_TO_STRING(Type)            \
template <typename Sink>                          \
inline void to_string(FormatSinkAdapter<Sink> out, Type n) \
{                                                 \
	details::IntegerFormater formater;            \
	out.append(formater.format(n));               \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_INTEGER_TO_STRING)

#undef LIGHTS_INTEGER_TO_STRING


template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, float n)
{
	char buf[std::numeric_limits<float>::max_exponent10 + 1 +
		std::numeric_limits<float>::digits10 + 1];
	int writen = std::sprintf(buf, "%f", n);
	out.append({buf, static_cast<std::size_t>(writen)});
}

template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, double n)
{
	// 100 is the max exponent10 of double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%f", n);
	out.append({buf, static_cast<std::size_t>(writen)});
}

template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, long double n)
{
	// 100 is the max exponent10 of long double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<long double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%Lf", n);
	out.append({buf, static_cast<std::size_t>(writen)});
}

template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, ErrorNumber error_no)
{
	char buf[ENV_MAX_ERROR_STR_LEN];
	const char* result = env_strerror(error_no.value, buf, ENV_MAX_ERROR_STR_LEN);
	out.append(result);
}

template <typename Sink>
void to_string(FormatSinkAdapter<Sink> out, Timestamp timestamp)
{
	std::tm tm;
	env_localtime(&timestamp.value, &tm);

	out << static_cast<unsigned>(tm.tm_year + 1900) << '-';
	// Why not use pad, because pad will it more complex and slow.
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_mon + 1)) << '-';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_mday)) << ' ';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_hour)) << ':';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_min)) << ':';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_sec));
}

/**
 * Creates a new spec with padding parameter.
 */
template <typename Integer, typename Tag>
inline IntegerFormatSpec<Integer, Tag> pad(IntegerFormatSpec<Integer, Tag> spec, char fill, int width)
{
	spec.width = width;
	spec.fill = fill;
	return spec;
}

/**
 * Creates a spec with padding parameter.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::DecimalSpecTag> pad(Integer n, char fill, int width)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	IntegerFormatSpec<Integer, details::DecimalSpecTag> spec = {
		n,
		details::DecimalSpecTag(),
		width,
		fill
	};
	return spec;
}

/**
 * Creates a binary spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::BinarySpecTag> binary(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::BinarySpecTag> { n };
}

/**
 * Converts integer to binary string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::BinarySpecTag> spec)
{
	details::BinaryFormater<Integer>::format(out, spec);
}


/**
 * Creates a octal spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::OctalSpecTag> octal(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::OctalSpecTag> { n };
}

/**
 * Converts integer to binary string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::OctalSpecTag> spec)
{
	details::OctalFormater<Integer>::format(out, spec);
}


/**
 * Creates a hex lower case spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> hex_lower_case(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> { n };
}

/**
 * Converts integer to hex lower case string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> spec)
{
	details::HexFormater<Integer>::format(out, spec);
}

/**
 * Creates a hex upper case spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> hex_upper_case(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> { n };
}

/**
 * Converts integer to hex upper case string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> spec)
{
	details::HexFormater<Integer>::format(out, spec);
}

/**
 * Converts integer to decimal string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::DecimalSpecTag> spec)
{
	details::IntegerFormater formater;
	StringView str = formater.format(spec.value);

	if (spec.width != INVALID_INDEX && str.length() < static_cast<std::size_t>(spec.width))
	{
		out.append(static_cast<std::size_t>(spec.width) - str.length(), spec.fill);
	}
	out.append(str);
}


/**
 * Appends the @c arg to the end of @c out. It'll invoke @c to_string()
 * Aim to support append not string type to the end of @c out.
 * @param out    A FormatSinkAdapter.
 * @param value  Any type value.
 */
template <typename Sink, typename T>
inline void append(FormatSinkAdapter<Sink> out, const T& value)
{
	to_string(out, value);
}

template <typename Sink>
inline void append(FormatSinkAdapter<Sink> out, char* str)
{
	out.append(str);
}

template <typename Sink>
inline void append(FormatSinkAdapter<Sink> out, const char* str)
{
	out.append(str);
}

template <typename Sink>
inline void append(FormatSinkAdapter<Sink> out, const std::string& str)
{
	out.append(str);
}

template <typename Sink>
inline void append(FormatSinkAdapter<Sink> out, StringView str)
{
	out.append(str);
}


/**
 * Inserts value into the sink. It'll invoke @c append()
 * Aim to support user can define
 *   `FormatSinkAdapter<Sink> operator<< (FormatSinkAdapter<Sink> out, const T& value)`
 * to format with user type.
 * @param out    A FormatSinkAdapter.
 * @param value  Built-in type or user-defined type value.
 * @return FormatSinkAdapter.
 */
template <typename Sink, typename T>
inline FormatSinkAdapter<Sink> operator<< (FormatSinkAdapter<Sink> out, const T& value)
{
	append(out, value);
	return out;
}

/**
 * Uses for recursion of unpack arguments of @c write() when have not argument.
 */
template <typename Sink>
inline void write(FormatSinkAdapter<Sink> out, StringView fmt)
{
	out.append(fmt);
}

/**
 * Writes to the end of string that use @c fmt and @c args ...
 * @param sink  Output holder.
 * @param fmt   Formats string that use '{}' as placeholder.
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
void write(FormatSinkAdapter<Sink> out, StringView fmt, const Arg& value, const Args& ... args)
{
	std::size_t i = 0;
	for (; i < fmt.length(); ++i)
	{
		if (fmt[i] == '{' &&
			i + 1 < fmt.length() &&
			fmt[i+1] == '}')
		{
			break;
		}
	}

	out.append({fmt.data(), i});
	if (i < fmt.length())
	{
		append(out, value);
		fmt.move_forward(i + 2);
		write(out, fmt, args ...);
	}
}


enum WriterBufferSize: std::size_t
{
	WRITER_BUFFER_SIZE_SMALL = 100,
	WRITER_BUFFER_SIZE_MIDDLE = 500,
	WRITER_BUFFER_SIZE_LARGE = 1000,
	WRITER_BUFFER_SIZE_HUGE = 4000,
	WRITER_BUFFER_SIZE_DEFAULT = WRITER_BUFFER_SIZE_MIDDLE,
};


/**
 * TextWriter use to format text and have internal buffer to hold all format result.
 * @tparam buffer_size  Default size is @c WRITER_BUFFER_SIZE_DEFAULT.
 * @note If the internal buffer is full will have no effect, unless have already
 *       set full handler.
 */
template <std::size_t buffer_size = WRITER_BUFFER_SIZE_DEFAULT>
class TextWriter
{
public:
	using FullHandler = std::function<void(StringView)>;

	/**
	 * Basic append function to append a character.
	 * Appends a char to the end of internal buffer.
	 * @param ch  Character to append.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	void append(char ch)
	{
		if (can_append(sizeof(ch)))
		{
			m_buffer[m_length] = ch;
			++m_length;
		}
		else // Full
		{
			if (m_full_handler)
			{
				m_full_handler(str_view());
				clear();
				if (sizeof(ch) <= max_size())
				{
					append(ch);
				}
			}
		}
	}

	/**
	 * Basic append function to append len of characters.
	 * Appends the first len character string point to by str to the end of internal buffer.
	 * @param str  Points to the character string to append.
	 * @param len  Length of character to append.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	void append(StringView str)
	{
		if (can_append(str.length()))
		{
			copy_array(m_buffer + m_length, str.data(), str.length());
			m_length += str.length();
		}
		else // Have not enought space to hold all.
		{
			// Append to the remaining place.
			StringView part(str.data(), max_size() - m_length);
			append(part);
			str.move_forward(part.length());
			hand_for_full(str);
		}
	}

	/**
	 * Forwards to lights::write() function.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	template <typename Arg, typename ... Args>
	void write(StringView fmt, const Arg& value, const Args& ... args)
	{
		// Must add namespace scope limit or cannot find suitable function.
		lights::write(make_format_sink_adapter(*this), fmt, value, args ...);
	}

	/**
	 * Forward to lights::write() function.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	void write(StringView fmt)
	{
		lights::write(make_format_sink_adapter(*this), fmt);
	}

	/**
	 * Inserts integer to internal buffer.
	 * @return The reference of this object.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
#define LIGHTS_TEXT_WRITER_APPEND_INTEGER(Type)           \
	TextWriter& operator<< (Type n)                       \
	{                                                       \
		auto len = details::format_need_space(n);           \
		if (can_append(len))                                \
		{                                                   \
			details::format_integer(n, m_buffer + m_length + len); \
			m_length += len;                                \
		}                                                   \
		return *this;                                       \
	}

	LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_TEXT_WRITER_APPEND_INTEGER)

#undef LIGHTS_TEXT_WRITER_APPEND_INTEGER

	/**
	 * Forwards to lights::operater<<() function.
	 * @param value  User-defined type value.
	 * @return The reference of this object.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	template <typename T>
	TextWriter& operator<< (const T& value)
	{
		make_format_sink_adapter(*this) << value;
		return *this;
	}

	/**
	 * Returns a pointer to null-terminated character array that store in internal.
	 */
	const char* c_str() const
	{
		const_cast<TextWriter*>(this)->m_buffer[m_length] = '\0';
		return m_buffer;
	}

	/**
	 * Returns a @c std::string that convert from internal buffer.
	 * @note This convertion will generate a data copy.
	 */
	std::string str() const
	{
		return str_view().to_string();
	}

	/**
	 * Returns a @c StringView of internal buffer.
	 * @note The return value is only valid when this object have no change
	 *       the area of return @c StringView.
	 */
	StringView str_view() const
	{
		return { m_buffer, m_length };
	}

	/**
	 * Returns the length of internal buffer.
	 */
	std::size_t length() const
	{
		return m_length;
	}

	/**
	 * Returns the length of internal buffer.
	 * @details It's same as @c length() function.
	 */
	std::size_t size() const
	{
		return m_length;
	}

	/**
	 * Sets the format result length to zero.
	 */
	void clear()
	{
		m_length = 0;
	}

	const FullHandler& get_full_handler() const
	{
		return m_full_handler;
	}

	/**
	 * Sets full handler to listen for internal buffer is full.
	 * @details After set full handler, this handler will be call when internal buffer
	 *          is full. And reset interal buffer and try to append argument.
	 */
	void set_full_handler(const FullHandler& full_handler)
	{
		m_full_handler = full_handler;
	}

	/**
	 * Returns the max size that format result can be.
	 */
	constexpr std::size_t max_size() const
	{
		return buffer_size - 1; // Remain a charater to hold null chareter.
	}

	/**
	 * Returns the internal buffer size.
	 */
	constexpr std::size_t capacity() const
	{
		return buffer_size;
	}

private:
	bool can_append(std::size_t len)
	{
		return m_length + len <= max_size();
	}

	void hand_for_full(StringView str);

	std::size_t m_length = 0;
	char m_buffer[buffer_size];
	FullHandler m_full_handler;
};


template <std::size_t buffer_size>
void TextWriter<buffer_size>::hand_for_full(StringView str)
{
	if (m_full_handler)
	{
		m_full_handler(str_view());
		clear();
		if (str.length() <= max_size())
		{
			append(str);
		}
		else // Have not enought space to hold all.
		{
			while (str.length())
			{
				std::size_t append_len = (str.length() <= max_size()) ? str.length() : max_size();
				StringView part(str.data(), append_len);
				append(part);
				if (append_len == max_size())
				{
					m_full_handler(str_view());
					clear();
				}
				str.move_forward(append_len);
			}
		}
	}
}


template <>
template <std::size_t buffer_size>
class FormatSinkAdapter<TextWriter<buffer_size>>
{
public:
	explicit FormatSinkAdapter(TextWriter<buffer_size>& sink) : m_sink(sink) {}

	void append(char ch)
	{
		m_sink.append(ch);
	}

	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	void append(StringView str)
	{
		m_sink.append(str);
	}

	TextWriter<buffer_size>& get_internal_sink()
	{
		return m_sink;
	}

private:
	TextWriter<buffer_size>& m_sink;
};


/**
 * Uses @c TextWriter member function to format integer to speed up.
 */
#define LIGHTS_TEXT_WRITER_TO_STRING(Type) \
template <std::size_t buffer_size> \
inline void to_string(FormatSinkAdapter<TextWriter<buffer_size>> out, Type n) \
{ \
	out.get_internal_sink() << n; \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_TEXT_WRITER_TO_STRING)

#undef LIGHTS_TEXT_WRITER_TO_STRING


/**
 * Formats string that use @c fmt and @c args ...
 * @param fmt   Formats string that use '{}' as placeholder.
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
inline std::string format(StringView fmt, const Args& ... args)
{
	std::string out;
	write(make_format_sink_adapter(out), fmt, args ...);
	return out;
}

} // namespace lights

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


/**
 * IntegerFormatSpec description interger how to be format.
 */
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

/**
 * Returns current error number.
 */
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

/**
 * Returns current error number.
 */
inline Timestamp current_timestamp()
{
	return Timestamp(std::time(nullptr));
}


/**
 * Format sink to provide same interface.
 * It's light weight and can be use as pass by value.
 * @tparam Backend  Store all append data.
 * @note This class template cannot to be use, and must to be
 *       explicit specialization and implement the method by user.
 */
template <typename Backend>
class FormatSink
{
public:
	/**
	 * Creates format sink.
	 */
	explicit FormatSink(Backend& backend) = delete;

	/**
	 * Appends char to backend.
	 */
	void append(char ch)
	{
		m_backend.append(ch);
	};

	/**
	 * Appends multiple same char to backend.
	 */
	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	/**
	 * Appends string to backend.
	 */
	void append(StringView str)
	{
		while (str.length() != 0)
		{
			this->append(*str.data());
			str.move_forward(1);
		}
	}

private:
	Backend& m_backend;
};

/**
 * Helper function to create format sink.
 */
template <typename Backend>
inline FormatSink<Backend> make_format_sink(Backend& backend)
{
	return FormatSink<Backend>(backend);
}

/**
 * Explicit tempate specialization of std::string.
 */
template <>
class FormatSink<std::string>
{
public:
	/**
	 * Creates format sink.
	 */
	explicit FormatSink(std::string& backend) : m_backend(backend) {}

	/**
	 * Appends char to backend.
	 */
	void append(char ch)
	{
		m_backend.push_back(ch);
	}

	/**
	 * Appends multiple same char to backend.
	 */
	void append(std::size_t num, char ch)
	{
		m_backend.append(num, ch);
	}

	/**
	 * Appends string to backend.
	 */
	void append(StringView str)
	{
		m_backend.append(str.data(), str.length());
	}

private:
	std::string& m_backend;
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


/**
 * IntegerFormater support to convert integer to string.
 */
class IntegerFormater
{
public:
	/**
	 * Creates integer formater.
	 */
	IntegerFormater()
	{
		m_buf[sizeof(m_buf) - 1] = '\0';
	}

	/**
	 * Converts integer to string.
	 * @param num  Any type of integer.
	 * @return Returns internal string result.
	 */
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


/**
 * Writes number and pad with two digit.
 */
template <typename Backend>
inline FormatSink<Backend> write_2_digit(FormatSink<Backend> sink, unsigned num)
{
	if (num >= 10)
	{
		sink << num;
	}
	else
	{
		sink << '0' << num;
	}
	return sink;
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


/**
 * HexConvertHandler is general hex convert handler.
 */
template <typename Tag>
struct HexConvertHandler;

/**
 * This class include handler that conver hex to lower case char.
 */
template <>
struct HexConvertHandler<HexSpecLowerCaseTag>
{
	using Handler = char(*)(char);
	static constexpr Handler handler = to_hex_lower_case_char;
};

/**
 * This class include handler that conver hex to upper case char.
 */
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
	template <typename Backend>
	static void format(FormatSink<Backend> sink,
					   IntegerFormatSpec<UnsignedInteger, details::BinarySpecTag> spec,
					   bool negative = false);
};

template <typename UnsignedInteger>
template <typename Backend>
void BinaryFormater<UnsignedInteger, false>::format(FormatSink<Backend> sink,
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
		sink.append(spec.width - width, spec.fill);
	}

	if (negative)
	{
		sink.append('-');
	}

	UnsignedInteger mask = 1ul << (num - 1);
	while (mask != 0)
	{
		char ch = to_binary_char(spec.value & mask);
		sink.append(ch);
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
	template <typename Backend>
	static void format(FormatSink<Backend> sink,
					   IntegerFormatSpec<UnsignedInteger, details::OctalSpecTag> spec,
					   bool negative = false);
};

template <typename UnsignedInteger>
template <typename Backend>
void OctalFormater<UnsignedInteger, false>::format(FormatSink<Backend> sink,
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
		sink.append(spec.width - width, spec.fill);
	}

	if (negative)
	{
		sink.append('-');
	}
	sink.append({ptr, num});
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
	template <typename Backend, typename Tag>
	static void format(FormatSink<Backend> sink,
					   IntegerFormatSpec<UnsignedInteger, Tag> spec,
					   bool negative = false);
};

template <typename UnsignedInteger>
template <typename Backend, typename Tag>
void HexFormater<UnsignedInteger, false>::format(FormatSink<Backend> sink,
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
		sink.append(spec.width - width, spec.fill);
	}

	if (negative)
	{
		sink.append('-');
	}
	sink.append({ptr, num});
}


/**
 * Formater with signed integer.
 */
#define LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(formater_name)                          \
template <typename SignedInteger>                                                     \
class formater_name<SignedInteger, true>                                              \
{                                                                                     \
public:                                                                               \
	template <typename Backend, typename Tag>                                         \
	static void format(FormatSink<Backend> sink, IntegerFormatSpec<SignedInteger, Tag> spec) \
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
		formater_name<UnsignedInteger>::format(sink, new_spec, negative);             \
	}                                                                                 \
};

LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(BinaryFormater)
LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(OctalFormater)
LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER(HexFormater)

#undef LIGHTS_DETAILS_UNSIGNED_SPEC_FORMATER

} // namespace details


/**
 * Converts boolean to string and put to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, bool is)
{
	sink.append(is ? "true" : "false");
}

/**
 * Puts char to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, char ch)
{
	sink.append(ch);
}

/**
 * Formats all type of integer to string and put to format sink.
 * @details Why must explicit specialization it? Because if not do that, SFINAE will
 *          pass user-defined type into this template function and cause compile error.
 */
#define LIGHTS_INTEGER_TO_STRING(Type)            \
template <typename Backend>                       \
inline void to_string(FormatSink<Backend> sink, Type n) \
{                                                 \
	details::IntegerFormater formater;            \
	sink.append(formater.format(n));              \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_INTEGER_TO_STRING)

#undef LIGHTS_INTEGER_TO_STRING


/**
 * Converts float to string and put to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, float n)
{
	char buf[std::numeric_limits<float>::max_exponent10 + 1 +
		std::numeric_limits<float>::digits10 + 1];
	int writen = std::sprintf(buf, "%f", n);
	sink.append({buf, static_cast<std::size_t>(writen)});
}

/**
 * Converts double to string and put to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, double n)
{
	// 100 is the max exponent10 of double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%f", n);
	sink.append({buf, static_cast<std::size_t>(writen)});
}

/**
 * Converts long double to string and put to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, long double n)
{
	// 100 is the max exponent10 of long double that can format in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	char buf[100 + 1 + std::numeric_limits<long double>::digits10 + 1];
	int writen = std::snprintf(buf, sizeof(buf), "%Lf", n);
	sink.append({buf, static_cast<std::size_t>(writen)});
}


/**
 * Converts error number to string and put to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, ErrorNumber error_no)
{
	char buf[env::MAX_ERROR_STR_LEN];
	const char* result = env::strerror(error_no.value, buf, env::MAX_ERROR_STR_LEN);
	sink.append(result);
}

/**
 * Converts timestamp to string and put to format sink.
 */
template <typename Backend>
void to_string(FormatSink<Backend> sink, Timestamp timestamp)
{
	std::tm tm;
	env::localtime(&timestamp.value, &tm);

	sink << static_cast<unsigned>(tm.tm_year + 1900) << '-';
	// Why not use pad, because pad will it more complex and slow.
	details::write_2_digit(sink, static_cast<unsigned>(tm.tm_mon + 1)) << '-';
	details::write_2_digit(sink, static_cast<unsigned>(tm.tm_mday)) << ' ';
	details::write_2_digit(sink, static_cast<unsigned>(tm.tm_hour)) << ':';
	details::write_2_digit(sink, static_cast<unsigned>(tm.tm_min)) << ':';
	details::write_2_digit(sink, static_cast<unsigned>(tm.tm_sec));
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
 * @param sink  The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Backend, typename Integer>
inline void to_string(FormatSink<Backend> sink, IntegerFormatSpec<Integer, details::BinarySpecTag> spec)
{
	details::BinaryFormater<Integer>::format(sink, spec);
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
 * @param sink  The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Backend, typename Integer>
inline void to_string(FormatSink<Backend> sink, IntegerFormatSpec<Integer, details::OctalSpecTag> spec)
{
	details::OctalFormater<Integer>::format(sink, spec);
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
 * @param sink  The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Backend, typename Integer>
inline void to_string(FormatSink<Backend> sink, IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> spec)
{
	details::HexFormater<Integer>::format(sink, spec);
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
 * @param sink  The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Backend, typename Integer>
inline void to_string(FormatSink<Backend> sink, IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> spec)
{
	details::HexFormater<Integer>::format(sink, spec);
}

/**
 * Converts integer to decimal string.
 * @param sink  The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Backend, typename Integer>
inline void to_string(FormatSink<Backend> sink, IntegerFormatSpec<Integer, details::DecimalSpecTag> spec)
{
	details::IntegerFormater formater;
	StringView str = formater.format(spec.value);

	if (spec.width != INVALID_INDEX && str.length() < static_cast<std::size_t>(spec.width))
	{
		sink.append(static_cast<std::size_t>(spec.width) - str.length(), spec.fill);
	}
	sink.append(str);
}


/**
 * Convert value to string and append to format sink. It'll invoke @c to_string()
 * Aim to support append not string type to format sink.
 * @param sink   A FormatSink.
 * @param value  Any type value.
 */
template <typename Backend, typename T>
inline void append(FormatSink<Backend> sink, const T& value)
{
	to_string(sink, value);
}

/**
 * Appends string to format sink.
 */
template <typename Backend>
inline void append(FormatSink<Backend> sink, char* str)
{
	sink.append(str);
}

/**
 * Appends string to format sink.
 */
template <typename Backend>
inline void append(FormatSink<Backend> sink, const char* str)
{
	sink.append(str);
}

/**
 * Appends string to format sink.
 */
template <typename Backend>
inline void append(FormatSink<Backend> sink, const std::string& str)
{
	sink.append(str);
}

/**
 * Appends string to format sink.
 */
template <typename Backend>
inline void append(FormatSink<Backend> sink, StringView str)
{
	sink.append(str);
}


/**
 * Inserts value into the sink. It'll invoke @c append()
 * Aim to support user can define
 *   `FormatSink<Backend> operator<< (FormatSink<Backend> out, const T& value)`
 * to format with user type.
 * @param sink   A FormatSink.
 * @param value  Built-in type or user-defined type value.
 * @return FormatSink.
 */
template <typename Backend, typename T>
inline FormatSink<Backend> operator<< (FormatSink<Backend> sink, const T& value)
{
	append(sink, value);
	return sink;
}

/**
 * Uses for recursion of unpack arguments of @c write() when have not argument.
 */
template <typename Backend>
inline void write(FormatSink<Backend> sink, StringView fmt)
{
	sink.append(fmt);
}

/**
 * Writes to the end of string that use @c fmt and @c args ...
 * @param sink  Output holder.
 * @param fmt   Formats string that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @details If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `FormatSink<Backend> operator<< (FormatSink<Backend> sink, const T& value);`
 *         3) `void append(FormatSink<Backend> sink, const T& value);`
 *         4) `void to_string(FormatSink<Backend> sink, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2), 3) and 4) is optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use using declare when not in lights namespace.
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3), 4) and 1).
 * @note 2), 3) and 4) must define in lights namespace to enable use placeholder to format
 *          you user-defined type.
 */
template <typename Backend, typename Arg, typename ... Args>
void write(FormatSink<Backend> sink, StringView fmt, const Arg& value, const Args& ... args)
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

	sink.append({fmt.data(), i});
	if (i < fmt.length())
	{
		sink << value;
		fmt.move_forward(i + 2);
		write(sink, fmt, args ...);
	}
}


/**
 * WriterBufferSize is enum of writer buffer.
 */
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
 * Specify buffer by manual can easy to limit the output.
 * @note If the internal buffer is full will have no effect, unless have already
 *       set full handler.
 */
class TextWriter
{
public:
	using FullHandler = std::function<void(StringView)>;

	/**
	 * Create text writer.
	 * @param write_target If write target is not specify, will use default write target with default size.
	 */
	TextWriter(String write_target = invalid_string()):
		m_use_default_buffer(!is_valid(write_target)),
		m_buffer(is_valid(write_target) ? write_target.data() : new char[WRITER_BUFFER_SIZE_DEFAULT]),
		m_capacity(is_valid(write_target) ? write_target.length() : WRITER_BUFFER_SIZE_DEFAULT)
	{}

	/**
	 * Destroys text writer.
	 */
	~TextWriter()
	{
		if (m_use_default_buffer)
		{
			delete[] m_buffer;
		}
	}

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
			handle_full(ch);
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
			handle_not_enougth_space(str);
		}
	}

	/**
	 * Forwards to lights::write() function.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	template <typename Arg, typename ... Args>
	void write(StringView fmt, const Arg& value, const Args& ... args);

	/**
	 * Forward to lights::write() function.
	 * @note If the internal buffer is full will have no effect, unless have already
	 *       set full handler.
	 */
	void write(StringView fmt);

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
		make_format_sink(*this) << value;
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
	std::string std_string() const
	{
		return string_view().to_std_string();
	}

	/**
	 * Returns a @c StringView of internal buffer.
	 * @note The return value is only valid when this object have no change
	 *       the area of return @c StringView.
	 */
	StringView string_view() const
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
	std::size_t max_size() const
	{
		return m_capacity - 1; // Remain a character to hold null chareter.
	}

	/**
	 * Returns the internal buffer size.
	 */
	std::size_t capacity() const
	{
		return m_capacity;
	}

private:
	bool can_append(std::size_t len)
	{
		return m_length + len <= max_size();
	}

	void handle_full(char ch);

	void handle_not_enougth_space(StringView str);

	void handle_full(StringView str);

	bool m_use_default_buffer;
	char* m_buffer;
	std::size_t m_length = 0;
	std::size_t m_capacity;
	FullHandler m_full_handler;
};


/**
 * FormatSink for TextWriter.
 */
template <>
class FormatSink<TextWriter>
{
public:
	/**
	 * Creates format sink.
	 */
	explicit FormatSink(TextWriter& backend) : m_backend(backend) {}

	/**
	 * Appends char to backend.
	 */
	void append(char ch)
	{
		m_backend.append(ch);
	}

	/**
	 * Appends multiple same char to backend.
	 */
	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	/**
	 * Appends string to backend.
	 */
	void append(StringView str)
	{
		m_backend.append(str);
	}

	/**
	 * Gets internal backend.
	 */
	TextWriter& get_internal_backend()
	{
		return m_backend;
	}

private:
	TextWriter& m_backend;
};


template <typename Arg, typename ... Args>
inline void TextWriter::write(StringView fmt, const Arg& value, const Args& ... args)
{
	// Must add namespace scope limit or cannot find suitable function.
	lights::write(make_format_sink(*this), fmt, value, args ...);
}

/**
 * @note Must ensure the specialization of FormatSink is declere before use.
 */
inline void TextWriter::write(StringView fmt)
{
	lights::write(make_format_sink(*this), fmt);
}


/**
 * Uses @c TextWriter member function to format integer to speed up.
 */
#define LIGHTS_TEXT_WRITER_TO_STRING(Type) \
inline void to_string(FormatSink<TextWriter> sink, Type n) \
{ \
	sink.get_internal_backend() << n; \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_TEXT_WRITER_TO_STRING)

#undef LIGHTS_TEXT_WRITER_TO_STRING


/**
 * It's workaroud way of make sure arguments are expanded.
 */
#define LIGHTS_CONCAT_IMPL(a, b) a##b
#define LIGHTS_CONCAT(a, b) LIGHTS_CONCAT_IMPL(a, b)

/**
 * Create a text writer with a buffer that in statck.
 */
#define LIGHTS_TEXT_WRITER(name, buffer_size) \
	char LIGHTS_CONCAT(name##_write_target_, __LINE__)[buffer_size]; \
	lights::TextWriter name({LIGHTS_CONCAT(name##_write_target_, __LINE__), buffer_size});

/**
 * Create a text writer with default size.
 */
#define LIGHTS_DEFAULT_TEXT_WRITER(name) \
	LIGHTS_TEXT_WRITER(name, lights::WRITER_BUFFER_SIZE_DEFAULT)


/**
 * Formats string that use @c fmt and @c args ...
 * @param fmt   Formats string that use '{}' as placeholder.
 * @param args  Variadic arguments that can be any type.
 * @return Formated string.
 * @details If args is user type, it must have a user function as
 *         1) `std::ostream& operator<< (std::ostream& out, const T& value);`
 *         2) `FormatSink<Backend> operator<< (FormatSink<Backend> sink, const T& value);`
 *         3) `void append(FormatSink<Backend> sink, const T& value);`
 *         4) `void to_string(FormatSink<Backend> sink, const T& value);`
 *       1) General way to use with @c std::ostream to format.
 *       2), 3) and 4) is optimize way with format.
 *          The implementation can use insertion operator (<<)
 *          between @c sink and @c value. But must use using declare when not in lights namespace.
 *            `using lights::operator<<`.
 *          After implement the user function, it also can be use
 *          in another user function as insertion operator.
 *       If all user function are implemented, the priority is 2), 3), 4) and 1).
 * @note 2), 3) and 4) must define in lights namespace to enable use placeholder to format
 *          you user-defined type.
 */
template <typename... Args>
inline std::string format(StringView fmt, const Args& ... args)
{
	std::string backend;
	write(make_format_sink(backend), fmt, args ...);
	return backend;
}

} // namespace lights

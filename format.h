/**
 * format.h
 * @author wherewindblow
 * @date   Dec 06, 2016
 */

#pragma once

#include <cstddef>
#include <cerrno>
#include <ctime>
#include <cstring>
#include <string>
#include <limits>
#include <sstream>
#include <functional>

#include <errno.h>

#include "config.h"


namespace lights {

#define LIGHTS_IMPLEMENT_SIGNED_FUNCTION(macro) \
	macro(std::int8_t)        \
	macro(std::int16_t)       \
	macro(std::int32_t)       \
	macro(std::int64_t)

#define LIGHTS_IMPLEMENT_UNSIGNED_FUNCTION(macro) \
	macro(std::uint8_t)       \
	macro(std::uint16_t)      \
	macro(std::uint32_t)      \
	macro(std::uint64_t)

#define LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(macro) \
	LIGHTS_IMPLEMENT_SIGNED_FUNCTION(macro) \
	LIGHTS_IMPLEMENT_UNSIGNED_FUNCTION(macro)


// To set this type to signed char for convenient to assign to
// any type. Char is the smallest basic type and assign to other
// type will not lose data. And signed will ensure this value is -1
// not 255, -1 signed char fit into int will be -1, fit into signed
// int will be the biggest value of signed int. If unsigned fit
// into int will be 255 and unsigned int also is this value.
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
 * A wrapper aim to identify this is a error not an integer.
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
 * View of string, can reduce data copy.
 */
class StringView
{
public:
	StringView() = default;

	StringView(const char* str, std::size_t len) :
		data(str), length(len) {}

	StringView(const char* str) :
		data(str), length(std::strlen(str)) {}

	StringView(const std::string& str) :
		data(str.data()), length(str.length()) {}

	std::string to_string()
	{
		return std::string(data, length);
	}

	const char* data;
	std::size_t length;
};


template <typename Ostream>
Ostream& operator<< (Ostream& out, StringView view)
{
	out.write(view.data, view.length);
	return out;
}

/**
 * Adapter Sink to provide same interface.
 * It's light weight and can be use as pass by value.
 * @tparam Sink  To be write backend.
 * @note This class template cannot to be use, and must to be
 *       instantiation and implement the method by user.
 */
template <typename Sink>
class FormatSinkAdapter
{
public:
	FormatSinkAdapter(Sink& sink) = delete;

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

	void append(StringView view)
	{
		while (view.length != 0)
		{
			this->append(*view.data);
			++view.data;
			--view.length;
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


template <>
class FormatSinkAdapter<std::string>
{
public:
	FormatSinkAdapter(std::string& sink) : m_sink(sink) {}

	void append(char ch)
	{
		m_sink.push_back(ch);
	}

	void append(std::size_t num, char ch)
	{
		m_sink.append(num, ch);
	}

	void append(StringView view)
	{
		m_sink.append(view.data, view.length);
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
		m_string.append({s, static_cast<std::size_t>(n)});
		return n;
	}

private:
	FormatSinkAdapter<Sink>& m_string;
};


#ifdef LIGHTS_DETAILS_INTEGER_FORMATER_OPTIMIZE
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

#define LIGHTS_DETAILS_INTEGER_FORMATER_FORMAT_UNSIGNED(Type) \
	StringView format(Type n)                         \
	{                                                 \
		this->reset_state();                          \
		return this->format_unsigned(n, m_begin);     \
	}

	LIGHTS_IMPLEMENT_UNSIGNED_FUNCTION(LIGHTS_DETAILS_INTEGER_FORMATER_FORMAT_UNSIGNED)

#undef LIGHTS_DETAILS_INTEGER_FORMATER_FORMAT_UNSIGNED

#define LIGHTS_DETAILS_INTEGER_FORMATER_FORMAT_SIGNED(Type) \
	StringView format(Type n)                      \
	{                                               \
		this->reset_state();                        \
		return format_signed(n, m_begin);           \
	}

	LIGHTS_IMPLEMENT_SIGNED_FUNCTION(LIGHTS_DETAILS_INTEGER_FORMATER_FORMAT_SIGNED);

#undef LIGHTS_DETAILS_INTEGER_FORMATER_FORMAT_SIGNED

	// TODO Consider to optimize like fmt.
	template <typename Integer>
	static unsigned need_space(Integer n);

	template <typename Integer>
	static char* format(Integer n, char* output);

private:
	/**
	 * @note Format character backwards to @c output and @c output this pos is not use.
	 */
	template <typename UnsignedInteger>
	static char* format_unsigned(UnsignedInteger n, char* output);

	/**
	 * @note Format character backwards to @c output and @c output this pos is not use.
	 */
	template <typename SignedInteger>
	static char* format_signed(SignedInteger n, char* output)
	{
		auto absolute = static_cast<std::make_unsigned_t<SignedInteger>>(n);
		bool negative = n < 0;
		if (negative)
		{
			absolute = 0 - absolute;
		}
		char* start = format_unsigned(absolute, output);
		if (negative)
		{
			--start;
			*start = '-';
		}
		return start;
	}

	void reset_state()
	{
		m_begin = m_buf + sizeof(m_buf) - 1;
	}

	char m_buf[std::numeric_limits<std::uintmax_t>::digits10 + 1 + 1];
	char* m_begin;
};

template <typename Integer>
unsigned IntegerFormater::need_space(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");

	auto absolute = static_cast<std::make_unsigned_t<Integer>>(n);
	bool negative = n < 0;
	unsigned count = 0;
	if (negative)
	{
		absolute = 0 - absolute;
		++count;
	}

	if (absolute == 0)
	{
		count = 1;
	}
	else
	{
		while (absolute >= 100)
		{
			absolute /= 100;
			count += 2;
		}

		if (absolute < 10) // Single digit.
		{
			++count;
		}
		else // Double digits.
		{
			count += 2;
		}
	}
	return count;
}

template <typename Integer>
char* IntegerFormater::format(Integer n, char* output)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");

	auto absolute = static_cast<std::make_unsigned_t<Integer>>(n);
	bool negative = n < 0;
	if (negative)
	{
		absolute = 0 - absolute;
	}

	if (absolute == 0)
	{
		--output;
		*output = '0';
	}
	else
	{
#ifndef LIGHTS_DETAILS_INTEGER_FORMATER_OPTIMIZE
		while (n != 0)
			{
				--output;
				*output = '0' + static_cast<char>(n % 10);
				n /= 10;
			}
#else
		while (absolute >= 100)
		{
			auto index = absolute % 100 * 2;
			--output;
			*output = digists[index + 1];
			--output;
			*output = digists[index];
			absolute /= 100;
		}

		if (absolute < 10) // Single digit.
		{
			--output;
			*output = '0' + static_cast<char>(absolute);
		}
		else // Double digits.
		{
			auto index = absolute * 2;
			--output;
			*output = digists[index + 1];
			--output;
			*output = digists[index];
		}
#endif
	}

	if (negative)
	{
		--output;
		*output = '-';
	}
	return output;
}

template <typename UnsignedInteger>
char* IntegerFormater::format_unsigned(UnsignedInteger n, char* output)
{
	if (n == 0)
	{
		--output;
		*output = '0';
	}
	else
	{
#ifndef LIGHTS_DETAILS_INTEGER_FORMATER_OPTIMIZE
		while (n != 0)
			{
				--output;
				*output = '0' + static_cast<char>(n % 10);
				n /= 10;
			}
#else
		while (n >= 100)
		{
			auto index = n % 100 * 2;
			--output;
			*output = digists[index + 1];
			--output;
			*output = digists[index];
			n /= 100;
		}

		if (n < 10) // Single digit.
		{
			--output;
			*output = '0' + static_cast<char>(n);
		}
		else // Double digits.
		{
			auto index = n * 2;
			--output;
			*output = digists[index + 1];
			--output;
			*output = digists[index];
		}
#endif
	}
	return output;
}


class ErrorFormater
{
public:
	StringView format(int errer_no)
	{
		// Not use sys_nerr and sys_errlist directly, although the are easy to control.
		// Because sys_nerr may bigger that sys_errlist size and sys_errlist may have't
		// all string for errno. In the way will lead to segment fault.
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE  // XSI-compliant (posix) version.
		if (strerror_r(errer_no, m_buf, sizeof(m_buf)) == 0)
		{
			return m_buf;
		}
		else
		{
			return "Unkown error";
		}
#else // GNU-specific version, m_buf only use when it is unkown.
		return strerror_r(errer_no, m_buf, sizeof(m_buf));
#endif
	}

	StringView format(ErrorNumber errer_no)
	{
		return this->format(errer_no.value);
	}

	StringView format_current_error()
	{
		return this->format(errno);
	}

private:
	// 100 charater is enough to put all error string
	// on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2) (Englist Version).
	// In another languague version may have to change to largger to
	// hold all message.
	// In GNU-specific version 100 charater can hold all unkown error.
	char m_buf[100];
};

template <typename Sink>
FormatSinkAdapter<Sink> write_2_digit(FormatSinkAdapter<Sink> out, unsigned num)
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


struct BinarySpecTag {};
struct OctalSpecTag {};
struct DecimalSpecTag {};
struct HexSpecLowerCaseTag {};
struct HexSpecUpperCaseTag {};

/**
 * Convert integer to binary character.
 * @param value  Integer
 * @return Binary character
 */
template <typename T>
inline char to_binary_char(T value)
{
	return value ? '1' : '0';
}

/**
 * Convert integer to lower case hex character.
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
 * Convert integer to upper case hex character.
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

/**
 * To convert @c value to string in the end of @ sink
 * that use insertion operator with std::ostream and T.
 * Aim to support format with
 *   `std::ostream& operator<< (std::ostream& out, const T& value)`
 * @param out    Abstract string.
 * @param value  User type.
 */
template <typename Sink, typename T>
inline void to_string(FormatSinkAdapter<Sink> out, const T& value)
{
	details::StringBuffer<Sink> buf(out);
	std::ostream ostream(&buf);
	ostream << value;
}

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
	details::ErrorFormater formater;
	out.append(formater.format(error_no));
}

template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, Timestamp timestamp)
{
	std::tm tm;
	localtime_r(&timestamp.value, &tm);

	out << static_cast<unsigned>(tm.tm_year + 1900) << '-';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_mon + 1)) << '-';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_mday)) << ' ';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_hour)) << ':';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_min)) << ':';
	details::write_2_digit(out, static_cast<unsigned>(tm.tm_sec));
}

/**
 * Create a new spec with padding parameter.
 */
template <typename Integer, typename Tag>
inline IntegerFormatSpec<Integer, Tag> pad(IntegerFormatSpec<Integer, Tag> spec, char fill, int width)
{
	spec.width = width;
	spec.fill = fill;
	return spec;
}

/**
 * Create a spec with padding parameter.
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
 * Create a binary spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::BinarySpecTag> binary(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::BinarySpecTag> { n };
}

/**
 * Convert integer to binary string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::BinarySpecTag> spec)
{
	details::BinaryFormater<Integer>::format(out, spec);
}


/**
 * Create a octal spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::OctalSpecTag> octal(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::OctalSpecTag> { n };
}

/**
 * Convert integer to binary string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::OctalSpecTag> spec)
{
	details::OctalFormater<Integer>::format(out, spec);
}


/**
 * Create a hex lower case spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> hex_lower_case(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> { n };
}

/**
 * Convert integer to hex lower case string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::HexSpecLowerCaseTag> spec)
{
	details::HexFormater<Integer>::format(out, spec);
}

/**
 * Create a hex upper case spec of formate integer.
 * @note Integer only can use integer type.
 */
template <typename Integer>
inline IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> hex_upper_case(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");
	return IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> { n };
}

/**
 * Convert integer to hex upper case string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::HexSpecUpperCaseTag> spec)
{
	details::HexFormater<Integer>::format(out, spec);
}

/**
 * Convert integer to decimal string.
 * @param out   The output place to hold the converted string.
 * @param spec  A spec of format integer.
 */
template <typename Sink, typename Integer>
inline void to_string(FormatSinkAdapter<Sink> out, IntegerFormatSpec<Integer, details::DecimalSpecTag> spec)
{
	details::IntegerFormater formater;
	StringView view = formater.format(spec.value);

	if (spec.width != INVALID_INDEX && view.length < static_cast<std::size_t>(spec.width))
	{
		out.append(static_cast<std::size_t>(spec.width) - view.length, spec.fill);
	}
	out.append(view);
}


/**
 * Append the @c arg to the end of @c out. It'll invoke @c to_string
 * Aim to support append not char* and not std::string
 * to the end of @c out.
 * @param out    Abstract string.
 * @param value  Any type.
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
 * Insert value into the sink. It'll invoke @c append
 * Aim to support user can define
 *   `StringAdapter<Sink> operator<< (StringAdapter<Sink> out, const T& value)`
 * to format with user type
 * @param out    Abstract string.
 * @param value  Built-in type or user type.
 * @return StringAdapter.
 */
template <typename Sink, typename T>
inline FormatSinkAdapter<Sink> operator<< (FormatSinkAdapter<Sink> out, const T& value)
{
	append(out, value);
	return out;
}


template <typename Sink>
inline void write(FormatSinkAdapter<Sink> out, StringView fmt)
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
void write(FormatSinkAdapter<Sink> out, StringView fmt, const Arg& value, const Args& ... args)
{
	std::size_t i = 0;
	for (; i < fmt.length; ++i)
	{
		if (fmt.data[i] == '{' &&
			i + 1 < fmt.length &&
			fmt.data[i+1] == '}')
		{
			break;
		}
	}

	out.append({fmt.data, i});
	if (i < fmt.length)
	{
		append(out, value);
		StringView view(fmt.data + i + 2, fmt.length - i - 2);
		write(out, view, args ...);
	}
}


template <std::size_t buffer_size>
class BinaryStoreWriter;

template <std::size_t buffer_size>
inline void write(FormatSinkAdapter<BinaryStoreWriter<buffer_size>> out, StringView fmt)
{
}

// This explicit template specialization must declare before use it or cannot
// into this function.
template <std::size_t buffer_size, typename Arg, typename ... Args>
void write(FormatSinkAdapter<BinaryStoreWriter<buffer_size>> out,
		   StringView fmt, const Arg& value, const Args& ... args)
{
	std::size_t i = 0;
	for (; i < fmt.length; ++i)
	{
		if (fmt.data[i] == '{' &&
			i + 1 < fmt.length &&
			fmt.data[i+1] == '}')
		{
			break;
		}
	}

	if (i < fmt.length)
	{
		out.get_internal_sink().add_composed_type(value);
		StringView view(fmt.data + i + 2, fmt.length - i - 2);
		write(out, view, args ...);
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


template <std::size_t buffer_size = WRITER_BUFFER_SIZE_DEFAULT>
class TextWriter
{
public:
	using FullHandler = std::function<void(StringView)>;

	/**
	 * Basic append function to append a character.
	 * Append a char to the end of internal buffer.
	 * @param ch  Character to append.
	 * @note If the internal buffer is full will have no effect.
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
	 * Append the first len character string point to by str to the end of internal buffer.
	 * @param str  Point to the character string to append.
	 * @param len  Length of character to append.
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(StringView view)
	{
		if (can_append(view.length))
		{
			std::memcpy(m_buffer + m_length, view.data, view.length);
			m_length += view.length;
		}
		else // Have not enought space to hold all.
		{
			// Append to the remaining place.
			StringView part(view.data, max_size() - m_length);
			append(part);
			view.length -= (max_size() - m_length);

			hand_for_full(view);
		}
	}

	template <typename Arg, typename ... Args>
	void write(StringView fmt, const Arg& value, const Args& ... args)
	{
		// Must add namespace scope limit or cannot find suitable function.
		lights::write(make_format_sink_adapter(*this), fmt, value, args ...);
	}

#define LIGHTS_TEXT_WRITER_APPEND_INTEGER(Type)           \
	TextWriter& operator<< (Type n)                       \
	{                                                       \
		auto len = details::IntegerFormater::need_space(n); \
		if (can_append(len))                                \
		{                                                   \
			details::IntegerFormater::format(n, m_buffer + m_length + len); \
			m_length += len;                                \
		}                                                   \
		return *this;                                       \
	}

	LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_TEXT_WRITER_APPEND_INTEGER)

#undef LIGHTS_TEXT_WRITER_APPEND_INTEGER

	template <typename T>
	TextWriter& operator<< (const T& value)
	{
		make_format_sink_adapter(*this) << value;
		return *this;
	}

	const char* c_str()
	{
		m_buffer[m_length] = '\0';
		return m_buffer;
	}

	std::string str() const
	{
		return std::string(m_buffer, m_length);
	}

	StringView str_view() const
	{
		return { m_buffer, m_length };
	}

	std::size_t length() const
	{
		return m_length;
	}

	std::size_t size() const
	{
		return m_length;
	}

	void clear()
	{
		m_length = 0;
	}

	const FullHandler& get_full_handler() const
	{
		return m_full_handler;
	}

	void set_full_handler(const FullHandler& full_handler)
	{
		m_full_handler = full_handler;
	}

	/**
	 * Get max size can be.
	 */
	constexpr std::size_t max_size() const
	{
		return buffer_size - 1; // Remain a charater to hold null chareter.
	}

	constexpr std::size_t capacity() const
	{
		return buffer_size;
	}

private:
	bool can_append(std::size_t len)
	{
		return m_length + len <= max_size();
	}

	void hand_for_full(StringView view);

	std::size_t m_length = 0;
	char m_buffer[buffer_size];
	FullHandler m_full_handler;
};


template <std::size_t buffer_size>
void TextWriter<buffer_size>::hand_for_full(StringView view)
{
	if (m_full_handler)
	{
		m_full_handler(str_view());
		clear();
		if (view.length <= max_size())
		{
			append(view);
		}
		else // Have not enought space to hold all.
		{
			while (view.length)
			{
				std::size_t append_len = (view.length <= max_size()) ? view.length : max_size();
				StringView part(view.data, append_len);
				append(part);
				if (append_len == max_size())
				{
					m_full_handler(str_view());
					clear();
				}
				view.data += append_len;
				view.length -= append_len;
			}
		}
	}
}


template <>
template <std::size_t buffer_size>
class FormatSinkAdapter<TextWriter<buffer_size>>
{
public:
	FormatSinkAdapter(TextWriter<buffer_size>& sink) : m_sink(sink) {}

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

	void append(StringView view)
	{
		m_sink.append(view);
	}

	TextWriter<buffer_size>& get_internal_sink()
	{
		return m_sink;
	}

private:
	TextWriter<buffer_size>& m_sink;
};

#define LIGHTS_TEXT_WRITER_TO_STRING(Type) \
template <std::size_t buffer_size> \
inline void to_string(FormatSinkAdapter<TextWriter<buffer_size>> out, Type n) \
{ \
	out.get_internal_sink() << n; \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_TEXT_WRITER_TO_STRING)

#undef LIGHTS_TEXT_WRITER_TO_STRING


enum class BinaryTypeCode: std::uint8_t
{
	INVALID = 0,
	BOOL = 1,
	CHAR = 2,
	STRING = 3,
	INT8_T = 4,
	UINT8_T = 5,
	INT16_T = 6,
	UINT16_T = 7,
	INT32_T = 8,
	UINT32_T = 9,
	INT64_T = 10,
	UINT64_T = 11,
	COMPOSED_TYPE  = 12,
	MAX
};

inline BinaryTypeCode get_type_code(bool)
{
	return BinaryTypeCode::BOOL;
}

inline BinaryTypeCode get_type_code(char)
{
	return BinaryTypeCode::CHAR;
}

inline BinaryTypeCode get_type_code(const char*)
{
	return BinaryTypeCode::STRING;
}

template <typename T>
inline BinaryTypeCode get_type_code(T)
{
	int offset;
	switch (std::numeric_limits<std::make_unsigned_t<T>>::digits)
	{
		case 8:
			offset = 0;
			break;
		case 16:
			offset = 1;
			break;
		case 32:
			offset = 2;
			break;
		case 64:
			offset = 3;
			break;
		default:
			offset = 0;
			break;
	}

	offset *= 2;
	offset += !std::numeric_limits<T>::is_signed ? 1 : 0;
	return static_cast<BinaryTypeCode>(static_cast<int>(BinaryTypeCode::INT8_T) + offset);
}


inline std::uint8_t get_type_width(BinaryTypeCode code)
{
	static std::uint8_t widths[] = {
		0, // Invalid.
		1, 1, // bool and char
		1,    // string.
		1, 1, // 8 bits
		2, 2, // 16 bits
		4, 4, // 32 bits
		8, 8, // 64 bits
		2,    // user-define composed type
	};

	std::uint8_t index = static_cast<std::uint8_t>(code);
	if (index < 0 || index >= static_cast<std::uint8_t>(BinaryTypeCode::MAX))
	{
		return widths[0];
	}
	else
	{
		return widths[index];
	}
}

template <std::size_t buffer_size = WRITER_BUFFER_SIZE_DEFAULT>
class BinaryStoreWriter
{
public:
	enum FormatComposedTypeState
	{
		NO_INIT,
		STARTED,
		ENDED
	};

	void append(char ch)
	{
		if (can_append(sizeof(BinaryTypeCode) + sizeof(ch)))
		{
			if (m_state == FormatComposedTypeState::STARTED)
			{
				++m_composed_member_num;
			}
			m_buffer[m_length++] = static_cast<std::uint8_t>(BinaryTypeCode::CHAR);
			m_buffer[m_length++] = static_cast<std::uint8_t>(ch);
		}
	}

	void append(StringView view)
	{
		if (can_append(view.length + sizeof(BinaryTypeCode) + sizeof(std::uint8_t)))
		{
			if (m_state == FormatComposedTypeState::STARTED)
			{
				++m_composed_member_num;
			}
			m_buffer[m_length++] = static_cast<std::uint8_t>(BinaryTypeCode::STRING);
			m_buffer[m_length++] = static_cast<std::uint8_t>(view.length);
			std::memcpy(m_buffer + m_length, view.data, view.length);
			m_length += view.length;
		}
	}

#define LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER(Type) \
	BinaryStoreWriter& operator<< (Type n) \
	{ \
		BinaryTypeCode type_code = get_type_code(n); \
		if (can_append(sizeof(BinaryTypeCode) + get_type_width(type_code))) \
		{ \
			if (m_state == FormatComposedTypeState::STARTED) \
			{ \
				++m_composed_member_num;\
			} \
			m_buffer[m_length++] = static_cast<std::uint8_t>(type_code); \
			Type* p = reinterpret_cast<Type *>(&m_buffer[m_length]); \
			*p = n; \
			m_length += get_type_width(type_code); \
		} \
		return *this; \
	}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER)

#undef LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER

	/**
	 * It's only for write format to call and easy to restore.
	 */
	template <typename T>
	void add_composed_type(const T& value)
	{
		if (m_state == FormatComposedTypeState::STARTED) // Reduce recursion.
		{
			make_format_sink_adapter(*this) << value;
		}
		else
		{
			m_composed_member_num = 0;
			std::uint8_t* type = m_buffer + m_length;
			auto composed_member_num = reinterpret_cast<std::uint16_t*>(m_buffer + m_length + sizeof(BinaryTypeCode));
			const auto composed_header_len = sizeof(BinaryTypeCode) + sizeof(std::uint16_t) / sizeof(std::uint8_t);
			m_length += composed_header_len;
			m_state = FormatComposedTypeState::STARTED;
			make_format_sink_adapter(*this) << value;
			m_state = FormatComposedTypeState::ENDED;

			if (m_composed_member_num > 1)
			{
				*type = static_cast<std::uint8_t>(BinaryTypeCode::COMPOSED_TYPE);
				*composed_member_num = m_composed_member_num;
			}
			else
			{
				std::size_t len = (m_buffer + m_length) - reinterpret_cast<std::uint8_t*>(composed_member_num + 1);
				std::memmove(type, composed_member_num + 1, len);
				m_length -= composed_header_len;
			}
		}
	}

	template <typename Arg, typename ... Args>
	void write(StringView fmt, const Arg& value, const Args& ... args)
	{
		lights::write(make_format_sink_adapter(*this), fmt, value, args ...);
	}

	std::uint8_t* data() const
	{
		return m_buffer;
	}

	const char* c_str() const
	{
		return str_view().data;
	}

	std::string str() const
	{
		return str_view().to_string();
	}

	StringView str_view() const
	{
		return { reinterpret_cast<const char*>(m_buffer), m_length };
	}

	std::size_t length() const
	{
		return m_length;
	}

	std::size_t size() const
	{
		return m_length;
	}

	void clear()
	{
		m_length = 0;
	}

	/**
	 * Get max size can be.
	 */
	constexpr std::size_t max_size() const
	{
		return buffer_size;
	}

	constexpr std::size_t capacity() const
	{
		return buffer_size;
	}

private:
	bool can_append(std::size_t len)
	{
		return m_length + len <= max_size();
	}

	std::size_t m_length = 0;
	std::uint8_t m_buffer[buffer_size];
	FormatComposedTypeState m_state = FormatComposedTypeState::NO_INIT;
	std::uint16_t m_composed_member_num = 0;
};

template <>
template <std::size_t buffer_size>
class FormatSinkAdapter<BinaryStoreWriter<buffer_size>>
{
public:
	FormatSinkAdapter(BinaryStoreWriter<buffer_size>& sink) : m_sink(sink) {}

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

	void append(StringView view)
	{
		m_sink.append(view);
	}

	BinaryStoreWriter<buffer_size>& get_internal_sink()
	{
		return m_sink;
	}

private:
	BinaryStoreWriter<buffer_size>& m_sink;
};


#define LIGHTS_BINARY_STORE_WRITER_TO_STRING(Type) \
template <std::size_t buffer_size> \
inline void to_string(FormatSinkAdapter<BinaryStoreWriter<buffer_size>> out, Type n) \
{ \
	out.get_internal_sink() << n; \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_BINARY_STORE_WRITER_TO_STRING)

#undef LIGHTS_BINARY_STORE_WRITER_TO_STRING


template <std::size_t buffer_size = WRITER_BUFFER_SIZE_DEFAULT>
class BinaryRestoreWriter
{
public:
	void append(char ch)
	{
		m_writer.append(ch);
	}

	void append(StringView view)
	{
		m_writer.append(view);
	}

	template <typename Arg, typename ... Args>
	void write_text(StringView fmt, const Arg& value, const Args& ... args)
	{
		// Must add namespace scope limit or cannot find suitable function.
		lights::write(make_format_sink_adapter(m_writer), fmt, value, args ...);
	}

	void write_binary(StringView fmt, const std::uint8_t* binary_store_args, std::size_t args_length);

	template <typename T>
	BinaryRestoreWriter& operator<< (const T& value)
	{
		make_format_sink_adapter(m_writer) << value;
		return *this;
	}

	const char* c_str()
	{
		return m_writer.c_str();
	}

	std::string str() const
	{
		return m_writer.str();
	}

	StringView str_view() const
	{
		return m_writer.str_view();
	}

	std::size_t length() const
	{
		return m_writer.length();
	}

	std::size_t size() const
	{
		return m_writer.size();
	}

	void clear()
	{
		m_writer.clear();
	}

	constexpr std::size_t max_size() const
	{
		return m_writer.max_size();
	}

	constexpr std::size_t capacity() const
	{
		return m_writer.capacity();
	}

private:
	/**
	 * Write a argument and get the width of argument.
	 */
	std::uint8_t write_argument(const uint8_t* binary_store_args);

	TextWriter<buffer_size> m_writer;
};

template <std::size_t buffer_size>
void BinaryRestoreWriter<buffer_size>::write_binary(StringView fmt, const std::uint8_t* binary_store_args, std::size_t args_length)
{
	if (args_length == 0)
	{
		append(fmt);
		return;
	}

	std::size_t i = 0;
	for (; i < fmt.length; ++i)
	{
		if (fmt.data[i] == '{' &&
			i + 1 < fmt.length &&
			fmt.data[i+1] == '}')
		{
			break;
		}
	}

	m_writer.append({fmt.data, i});
	if (i < fmt.length)
	{
		auto width = write_argument(binary_store_args);
		StringView view(fmt.data + i + 2, fmt.length - i - 2);
		write_binary(view, binary_store_args + width, args_length - width);
	}
}

template <std::size_t buffer_size>
std::uint8_t BinaryRestoreWriter<buffer_size>::write_argument(const std::uint8_t* binary_store_args)
{
	auto width = get_type_width(static_cast<BinaryTypeCode>(*binary_store_args));
	auto value_begin = binary_store_args + sizeof(BinaryTypeCode);
	switch (static_cast<BinaryTypeCode>(*binary_store_args))
	{
		case BinaryTypeCode::INVALID:
			break;
		case BinaryTypeCode::BOOL:
		{
			bool b = static_cast<bool>(*value_begin);
			m_writer << b;
			break;
		}
		case BinaryTypeCode::CHAR:
		{
			char ch = static_cast<char>(*value_begin);
			m_writer << ch;
			break;
		}
		case BinaryTypeCode::STRING:
		{
			width += binary_store_args[1];
			m_writer.append({reinterpret_cast<const char*>(&binary_store_args[2]), binary_store_args[1]});
			break;
		}
		case BinaryTypeCode::INT8_T:
		{
			auto p = reinterpret_cast<const int8_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT8_T:
		{
			auto p = value_begin;
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT16_T:
		{
			auto p = reinterpret_cast<const int16_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT16_T:
		{
			auto p = reinterpret_cast<const uint16_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT32_T:
		{
			auto p = reinterpret_cast<const int32_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT32_T:
		{
			auto p = reinterpret_cast<const uint32_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT64_T:
		{
			auto p = reinterpret_cast<const int64_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT64_T:
		{
			auto p = reinterpret_cast<const uint64_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::COMPOSED_TYPE:
		{
			auto member_num = reinterpret_cast<const uint16_t*>(value_begin);
			for (std::size_t i = 0; i < *member_num; ++i)
			{
				width += write_argument(binary_store_args + sizeof(BinaryTypeCode) + width);
			}
			break;
		}
		case BinaryTypeCode::MAX:
			break;
	}
	return width + sizeof(BinaryTypeCode);
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
inline std::string format(StringView fmt, const Args& ... args)
{
	std::string out;
	write(make_format_sink_adapter(out), fmt, args ...);
	return out;
}

} // namespace lights

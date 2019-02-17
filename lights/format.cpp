/*
 * format.cpp
 * @author wherewindblow
 * @date   Aug 13, 2017
 */

#include "format.h"


namespace lights {
namespace details {

// TODO Consider to optimize like fmt library.
template <typename Integer>
std::size_t format_need_space(Integer n)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");

	using UnsignedInteger = std::make_unsigned_t<Integer>;
	auto absolute = static_cast<UnsignedInteger>(n);
	bool negative = n < 0;
	unsigned count = 0;
	if (negative)
	{
		absolute = static_cast<UnsignedInteger>(0 - absolute);
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

#define LIGHTSIMPL_FORMAT_NEED_SPACE(Integer) \
template std::size_t format_need_space(Integer n);
LIGHTSIMPL_ALL_INTEGER_FUNCTION(LIGHTSIMPL_FORMAT_NEED_SPACE);
#undef LIGHTSIMPL_FORMAT_NEED_SPACE


template <typename Integer>
char* format_integer(Integer n, char* output)
{
	static_assert(std::is_integral<Integer>::value && "n only can be integer");

	using UnsignedInteger = std::make_unsigned_t<Integer>;
	auto absolute = static_cast<UnsignedInteger>(n);
	bool negative = n < 0;
	if (negative)
	{
		absolute = static_cast<UnsignedInteger>(0 - absolute);
	}

	if (absolute == 0)
	{
		--output;
		*output = '0';
	}
	else
	{
#ifndef LIGHTS_OPTIMIZE_INTEGER_FORMATER
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

#define LIGHTSIMPL_FORMAT_INTEGER(Integer) \
template char* format_integer(Integer n, char* output);
LIGHTSIMPL_ALL_INTEGER_FUNCTION(LIGHTSIMPL_FORMAT_INTEGER);
#undef LIGHTSIMPL_FORMAT_INTEGER

} // namespace details


TextWriter::TextWriter(String write_target) :
	m_use_default_buffer(!is_valid(write_target)),
	m_buffer(is_valid(write_target) ? write_target.data() : new char[WRITER_BUFFER_SIZE_DEFAULT]),
	m_length(0),
	m_capacity(is_valid(write_target) ? write_target.length() : WRITER_BUFFER_SIZE_DEFAULT),
	m_full_handler()
{}


TextWriter::TextWriter(const TextWriter& rhs)
{
	*this = rhs;
}


TextWriter::~TextWriter()
{
	if (m_use_default_buffer)
	{
		delete[] m_buffer;
	}
}


TextWriter& TextWriter::operator=(const TextWriter& rhs)
{
	if (&rhs != this)
	{
		if (m_use_default_buffer)
		{
			delete[] m_buffer;
		}

		m_use_default_buffer = rhs.m_use_default_buffer;
		m_buffer = rhs.m_use_default_buffer ? new char[WRITER_BUFFER_SIZE_DEFAULT] : rhs.m_buffer;
		m_length = rhs.m_length;
		m_capacity = rhs.m_capacity;
		m_full_handler = rhs.m_full_handler;
	}
	return *this;
}


void TextWriter::append(char ch)
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
			m_full_handler(string_view());
			clear();
			if (sizeof(ch) <= max_size())
			{
				append(ch);
			}
		}
	}
}


void TextWriter::append(StringView str)
{
	if (can_append(str.length()))
	{
		copy_array(m_buffer + m_length, str.data(), str.length());
		m_length += str.length();
	}
	else // Have not enough space to hold all.
	{
		// Append to the remaining place.
		StringView part(str.data(), max_size() - m_length);
		append(part);
		str.move_forward(part.length());

		// Handle for full situation.
		handle_full(str);
	}
}


void TextWriter::handle_full(StringView str)
{
	if (m_full_handler)
	{
		m_full_handler(string_view());
		clear();
		if (str.length() <= max_size())
		{
			append(str);
		}
		else // Have not enough space to hold all.
		{
			while (str.length())
			{
				std::size_t append_len = (str.length() <= max_size()) ? str.length() : max_size();
				StringView part(str.data(), append_len);
				append(part);
				if (append_len == max_size())
				{
					m_full_handler(string_view());
					clear();
				}
				str.move_forward(append_len);
			}
		}
	}
}


#define LIGHTSIMPL_TEXT_WRITER_INSERT_IMPL(Type)            \
	TextWriter& TextWriter::operator<< (Type n)             \
	{                                                       \
		auto len = details::format_need_space(n);           \
		if (can_append(len))                                \
		{                                                   \
			details::format_integer(n, m_buffer + m_length + len); \
			m_length += len;                                \
		}                                                   \
		return *this;                                       \
	}

LIGHTSIMPL_ALL_INTEGER_FUNCTION(LIGHTSIMPL_TEXT_WRITER_INSERT_IMPL)

#undef LIGHTSIMPL_TEXT_WRITER_INSERT_IMPL


void FormatSink<TextWriter>::append(std::size_t num, char ch)
{
	for (std::size_t i = 0; i < num; ++i)
	{
		this->append(ch);
	}
}

} // namespace lights

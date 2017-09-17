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

#define LIGHTS_DETAILS_FORMAT_NEED_SPACE(Integer) \
template std::size_t format_need_space(Integer n);
LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_DETAILS_FORMAT_NEED_SPACE);
#undef LIGHTS_DETAILS_FORMAT_NEED_SPACE


template <typename Integer>
char* format_integer(Integer n, char* output)
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

#define LIGHTS_DETAILS_FORMAT_INTEGER(Integer) \
template char* format_integer(Integer n, char* output);
LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_DETAILS_FORMAT_INTEGER);
#undef LIGHTS_DETAILS_FORMAT_INTEGER

} // namespace details


void TextWriter::hand_for_full(StringView str)
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

} // namespace lights

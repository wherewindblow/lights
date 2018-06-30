/**
 * example_format.cpp
 * @author wherewindblow
 * @date   Aug 23, 2017
 */

#include "example_format.h"

#include <lights/format.h>
#include <lights/file.h>


namespace lights {

template <>
class FormatSink<char*>
{
public:
	explicit FormatSink(char* sink, std::size_t max_size) :
		m_backend(sink), m_max_size(max_size) {}

	void append(char ch)
	{
		if (m_current_size + 1 <= m_max_size)
		{
			m_backend[m_current_size] = ch;
			++m_current_size;
		}
	}

	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	void append(lights::StringView str)
	{
		if (m_current_size + str.length() <= m_max_size)
		{
			lights::copy_array(m_backend + m_current_size, str.data(), str.length());
			m_current_size += str.length();
		}
	}

private:
	char* m_backend;
	std::size_t m_max_size;
	std::size_t m_current_size = 0;
};


namespace example {

void example_format()
{
	// Simple way to use.
	std::string msg = lights::format("start at {}:{}", __FILE__, __LINE__);
	lights::stdout_stream().write_line(msg);

	// Effective way to format.
	LIGHTS_DEFAULT_TEXT_WRITER(writer);
	writer.write("Current position is {}:{}", __FILE__, __LINE__);
	lights::stdout_stream().write_line(writer.string_view()); // Avoid create std::string and only return the internal string.
	lights::stdout_stream().write_line(writer.std_string()); // Return std::string.

	// Costom user-defined sink target.
	char external_buffer[500];
	lights::zero_array(external_buffer);
	lights::FormatSink<char*> format_sink(external_buffer, lights::size_of_array(external_buffer));
	lights::write(format_sink, "buffer size is {}", lights::size_of_array(external_buffer));
	lights::stdout_stream().write_line(external_buffer);

	// Format integer to with binary, octal and hex.
	int num = 100;
	std::string integer_spec = lights::format("decimal:{}, binary:{}, octal:{}, hex:{}",
											  num,
											  lights::binary(num),
											  lights::octal(num),
											  lights::hex_lower_case(num));
	lights::stdout_stream().write_line(integer_spec);

	// Format integer with padding.
	std::string padding = lights::format("origin:{}, after padding:{}, padding with hex:{}",
										 num,
										 lights::pad(num, '0', 5),
										 lights::pad(lights::hex_lower_case(num), '-', 5));
	lights::stdout_stream().write_line(padding);
}

} // namespace example
} // namespace lights

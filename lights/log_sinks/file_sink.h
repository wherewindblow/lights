/**
 * file_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <memory>

#include <stdio.h>
#include <string.h>

#include "../block_description.h"
#include "../file.h"
#include "../exception.h"

namespace lights {
namespace log_sinks {

class SimpleFileSink
{
public:
	/**
	 * Open the file with @c filename.
	 * @param filename  The file to be write.
	 */
	SimpleFileSink(StringView filename) :
		m_file(filename.data(), "ab+")
	{
		m_file.setbuf(nullptr);
	}

	void write(BufferView buffer)
	{
		m_file.write(buffer);
	}

private:
	FileStream m_file;
};


class SizeRotatingFileSink
{
public:
	SizeRotatingFileSink() = default;

#define LIGHTS_SINKS_INIT_MEMBER(exp) \
	if (can_init) \
	{             \
		exp;      \
	}             \
	else          \
	{             \
		assert(false && "SizeRotatingFileSink: Cannot initialize when have been end initialization"); \
	}

	/**
	 * Init file name format.
	 * @param name_format  Format string that use "{}" as a placehalder.
	 * @note This init is necessary.
	 */
	void init_name_format(const std::string& name_format)
	{
		LIGHTS_SINKS_INIT_MEMBER(m_name_format = name_format);
	}

	/**
	 * @note If not init max size, the biggest of std::size_t value is use.
	 */
	void init_max_size(std::size_t max_size)
	{
		LIGHTS_SINKS_INIT_MEMBER(m_max_size = max_size);
	}

	/**
	 * @note If not int max files, it'll not cycle to use file name.
	 */
	void init_max_files(std::size_t max_files)
	{
		LIGHTS_SINKS_INIT_MEMBER(m_max_files = max_files);
	}

#undef LIGHTS_SINKS_INIT_MEMBER

	void end_init()
	{
		can_init = false;
		this->rotate();
	}

	void write(BufferView buffer)
	{
		while (m_current_size + buffer.length() > m_max_size)
		{
			this->fill_remain();
			this->rotate();
		}
		m_file.write(buffer);
		m_current_size += buffer.length();
	}

private:
	void fill_remain();

	void rotate();

	bool can_init = true;
	std::string m_name_format;
	std::size_t m_max_size;
	std::size_t m_max_files = static_cast<std::size_t>(-1);
	FileStream m_file;
	std::size_t m_index = static_cast<std::size_t>(-1);
	std::size_t m_current_size;
};


class TimeRotatingFileSink
{
public:
	static constexpr std::time_t ONE_DAY_SECONDS = 3600 * 24;

	TimeRotatingFileSink(std::string name_format,
						 std::time_t duration = ONE_DAY_SECONDS,
						 std::time_t day_point = 0) :
		m_name_format(name_format),
		m_duration(duration),
		m_day_point(day_point)
	{
		if (day_point > ONE_DAY_SECONDS)
		{
			LIGHTS_THROW_EXCEPTION(InvalidArgument, format("day_point {} is bigger than ONE_DAY_SECONDS", day_point));
		}
		std::time_t now = std::time(nullptr);
		m_next_rotating_time = now;
		m_next_rotating_time -= m_next_rotating_time % ONE_DAY_SECONDS;
		m_next_rotating_time += day_point;

		if (m_next_rotating_time > now)
		{
			m_next_rotating_time -= m_duration;
		}

		rotate();
	}

	void write(BufferView buffer)
	{
		std::time_t now = std::time(nullptr);
		if (now >= m_next_rotating_time)
		{
			rotate();
		}
		m_file.write(buffer);
	}

	void rotate();

private:
	std::string m_name_format;
	std::time_t m_duration;
	std::time_t m_day_point;
	std::time_t m_next_rotating_time;
	FileStream m_file;
};

} // namespace log_sinks
} // namespace lights

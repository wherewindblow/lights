/**
 * file_sink.cpp
 * @author wherewindblow
 * @date   Jul 23, 2017
 */

#include "file_sink.h"

#include "../format.h"


namespace lights {
namespace log_sinks {

void SizeRotatingFileSink::fill_remain()
{
	if (m_current_size < m_max_size)
	{
		char zeros[500];
		zero_array(zeros);
		std::size_t fill_size = m_max_size - m_current_size;
		std::size_t times = fill_size / sizeof(zeros);
		std::size_t remainder = fill_size % sizeof(zeros);

		for (std::size_t i = 0; i < times; ++i)
		{
			m_file.write({zeros, sizeof(zeros)});
		}
		m_file.write({zeros, remainder});
	}
}


void SizeRotatingFileSink::rotate(std::size_t expect_size)
{
	bool appropriate = false;
	while (m_index + 1 < m_max_files)
	{
		++m_index;

		auto name = format(m_name_format, m_index);
		if (env::file_exists(name.c_str()))
		{
			continue;
		}

		bool cannot_use_previous = false;
		if (m_index != 0) // Have none previous file, because 0 is the first index.
		{
			// Try to use previous file if it have enought space to write a message.
			auto previous_name = format(m_name_format, m_index - 1);
			if (!env::file_exists(previous_name.c_str()))
			{
				if (m_file.is_open())
				{
					m_file.close();
				}
				m_file.open(previous_name, "ab+");
				if (m_file.size() + expect_size > m_max_size)
				{
					cannot_use_previous = true;
				}
				else
				{
					--m_index;
				}
			}
		}
		else
		{
			cannot_use_previous = true;
		}

		if (cannot_use_previous)
		{
			if (m_file.is_open())
			{
				m_file.close();
			}
			m_file.open(name, "ab+");
		}
		m_current_size = m_file.size();
		appropriate = true;
		break;
	}

	if (!appropriate)
	{
		auto first = format(m_name_format, 0);
		std::remove(first.c_str());

		for (std::size_t i = 1; i < m_max_files; ++i)
		{
			auto old_name = format(m_name_format, i);
			auto new_name = format(m_name_format, i - 1);
			std::rename(old_name.c_str(), new_name.c_str());
		}

		auto last = format(m_name_format, m_max_files - 1);
		if (m_file.is_open())
		{
			m_file.close();
		}
		m_file.open(last, "ab+");
		m_current_size = m_file.size();
	}

	m_msg_writer.set_write_target(&m_file);
}


TimeRotatingFileSink::TimeRotatingFileSink(std::string name_format, time_t duration, time_t day_point) :
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


void TimeRotatingFileSink::rotate()
{
	std::time_t time = std::time(nullptr);
	std::tm tm;
	localtime_r(&time, &tm);
	char buf[50];
	std::size_t buf_len = std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
	buf[buf_len] = '\0';

	std::string name = format(m_name_format, buf);

	if (m_file.is_open())
	{
		m_file.close();
	}
	m_file.open(name, "ab+");
	m_next_rotating_time += m_duration;
	m_msg_writer.set_write_target(&m_file);
}

} // namespace log_sinks
} // namespace lights

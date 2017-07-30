/**
 * file_sink.cpp
 * @author wherewindblow
 * @date   Jul 23, 2017
 */

#include "file_sink.h"

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
			m_file.write(zeros, sizeof(zeros));
		}
		m_file.write(zeros, remainder);
	}
}


void SizeRotatingFileSink::rotate()
{
	bool appropriate = false;
	while (m_index + 1 < m_max_files)
	{
		if (m_file.is_open())
		{
			m_file.close();
		}
		++m_index;
		auto name = format(m_name_format, m_index);
		m_file.open(name, "ab+");
		m_current_size = m_file.size();
		if (m_current_size < m_max_size)
		{
			appropriate = true;
			break;
		}
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
}

} // namespace log_sinks
} // namespace lights

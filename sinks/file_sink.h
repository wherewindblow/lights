/**
 * file_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <memory>

#include "../format.h"


namespace lights {
namespace sinks {

namespace details {

static void open_file_failure(const char* filename)
{
	std::string msg;
	msg.reserve(512);
	char buf[256];
	strerror_r(errno, buf, sizeof(buf));
	lights::write(msg, "Open \"{}\" failure: {}",
				  filename, buf);
	throw std::runtime_error(msg);
}

} // namespace details


class SimpleFileSink
{
public:
	/**
	 * Open the file with @c filename.
	 * @param filename  The file to be write.
	 * @throw Thrown std::runtime_error when open failure.
	 */
	SimpleFileSink(const char* filename) :
		m_file(std::fopen(filename, "ab+"))
	{
		if (m_file == nullptr)
		{
			details::open_file_failure(filename);
		}
	}

	SimpleFileSink(const std::string& filename) :
		SimpleFileSink(filename.c_str())
	{}

	void write(const char* str, std::size_t len)
	{
		std::fwrite(str, sizeof(char), len, m_file);
	}

private:
	std::FILE* m_file;
};


class RotatingFileSink
{
public:
	/**
	 * Open the file with @c filename.
	 * @param name_format  The file to be write.
	 * @throw Thrown std::runtime_error when open failure.
	 */
	RotatingFileSink(const char* name_format, std::size_t max_size) :
		m_name_format(name_format), m_max_size(max_size)
	{
		this->rotate();
	}

	RotatingFileSink(const std::string& name_format, std::size_t max_size) :
		m_name_format(name_format), m_max_size(max_size)
	{
		this->rotate();
	}

	void write(const char* str, std::size_t len)
	{
		while (m_current_size + len > m_max_size)
		{
			this->fill_remain();
			this->rotate();
		}
		std::fwrite(str, sizeof(char), len, m_file);
		m_current_size += len;
	}


private:
	void fill_remain()
	{
		if (m_current_size < m_max_size)
		{
			char zeros[500];
			std::size_t fill_size = m_max_size - m_current_size;
			std::size_t times = fill_size / sizeof(zeros);
			std::size_t remainder = fill_size % sizeof(zeros);
			std::memset(zeros, 0, sizeof(zeros));

			for (std::size_t i = 0; i < times; ++i)
			{
				std::fwrite(zeros, sizeof(char), sizeof(zeros), m_file);
			}
			std::fwrite(zeros, sizeof(char), remainder, m_file);
		}
	}

	void rotate()
	{
		++m_index;
		auto name = format(m_name_format.c_str(), m_index);
		m_file = std::fopen(name.c_str(), "ab+");
		if (m_file == nullptr)
		{
			details::open_file_failure(name.c_str());
		}
		else
		{
			std::fseek(m_file, 0, SEEK_END);
			m_current_size = static_cast<std::size_t>(std::ftell(m_file));
			if (m_current_size >= m_max_size)
			{
				this->rotate();
			}
			else
			{
				std::fseek(m_file, 0, SEEK_SET);
			}
		}
	}

	std::string m_name_format;
	const std::size_t m_max_size;
	std::FILE* m_file;
	int m_index = -1;
	std::size_t m_current_size;
};

} // namespace sinks
} // namespace lights

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
			std::string msg;
			msg.reserve(512);
			char buf[256];
			strerror_r(errno, buf, sizeof(buf));
			lights::write(msg, "Open \"{}\" failure: {}",
						  filename, buf);
			throw std::runtime_error(msg);
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

} // namespace sinks
} // namespace lights

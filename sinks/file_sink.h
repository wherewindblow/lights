/**
 * file_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <stdio.h>
#include <string.h>

#include <cstddef>
#include <memory>

#include "../format.h"


namespace lights {
namespace sinks {

namespace details {

enum class SeekWhence
{
	begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END
};


class FileStream
{
public:
	FileStream() = default;

	FileStream(const char* filename, const char* modes) :
		m_file(std::fopen(filename, modes))
	{
		if (m_file == nullptr)
		{
			FileStream::open_file_failure(filename);
		}
	}

	FileStream(const std::string& filename, const std::string& modes) :
		FileStream(filename.c_str(), modes.c_str())
	{}

	~FileStream()
	{
		if (m_file != nullptr)
		{
			this->close();
		}
	}

	void open(const char* filename, const char* modes)
	{
		m_file = std::fopen(filename, modes);
		if (m_file == nullptr)
		{
			FileStream::open_file_failure(filename);
		}
	}

	void open(const std::string& filename, const std::string& modes)
	{
		this->open(filename.c_str(), modes.c_str());
	}

	void reopen(const char* filename, const char* modes)
	{
		if (std::freopen(filename, modes, m_file) == nullptr)
		{
			FileStream::open_file_failure(filename);
		}
	}

	void reopen(const std::string& filename, const std::string& modes)
	{
		this->reopen(filename.c_str(), modes.c_str());
	}

	std::size_t read(char* buf, std::size_t len)
	{
		return std::fread(buf, sizeof(char), len, m_file);
	}

	std::size_t write(const char* buf, std::size_t len)
	{
		return std::fwrite(buf, sizeof(char), len, m_file);
	}

	void flush()
	{
		std::fflush(m_file);
	}

	bool eof()
	{
		return std::feof(m_file) != 0;
	}

	bool error()
	{
		return std::ferror(m_file) != 0;
	}

	void clear_error()
	{
		std::clearerr(m_file);
	}

	std::streamoff tell()
	{
		return ftello(m_file);
	}

	void seek(std::streamoff off, SeekWhence whence)
	{
		fseeko(m_file, off, static_cast<int>(whence));
	}

	void rewind()
	{
		std::rewind(m_file);
	}

	std::size_t size()
	{
		std::streamoff origin = this->tell();
		this->seek(0, SeekWhence::end);
		std::streamoff size = this->tell();
		this->seek(origin, SeekWhence::begin);
		return static_cast<std::size_t>(size);
	}

	void close()
	{
		std::fclose(m_file);
	}

private:
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

	std::FILE* m_file = nullptr;
};

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
		m_file(filename, "ab+")
	{}

	SimpleFileSink(const std::string& filename) :
		SimpleFileSink(filename.c_str())
	{}

	void write(const char* str, std::size_t len)
	{
		m_file.write(str, len);
	}

private:
	details::FileStream m_file;
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
		m_name_format(name_format),
		m_max_size(max_size),
		m_file(std::make_unique<details::FileStream>())
	{
		this->rotate();
	}

	RotatingFileSink(const std::string& name_format, std::size_t max_size) :
		RotatingFileSink(name_format.c_str(), max_size)
	{}

	void write(const char* str, std::size_t len)
	{
		while (m_current_size + len > m_max_size)
		{
			this->fill_remain();
			this->rotate();
		}
		m_file->write(str, len);
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
				m_file->write(zeros, sizeof(zeros));
			}
			m_file->write(zeros, remainder);
		}
	}

	void rotate()
	{
		while (true)
		{
			++m_index;
			auto name = format(m_name_format, m_index);
			m_file->open(name.c_str(), "ab+");
			m_current_size = m_file->size();
			if (m_current_size < m_max_size)
			{
				break;
			}
			else
			{
				m_file->close();
			}
		}
	}

	std::string m_name_format;
	const std::size_t m_max_size;
	std::unique_ptr<details::FileStream> m_file;
	int m_index = -1;
	std::size_t m_current_size;
};

} // namespace sinks
} // namespace lights

/**
 * file.h
 * @author wherewindblow
 * @date   Jun 17, 2017
 */

#pragma once

#include <cstdio>
#include <cassert>

#include "config.h"
#include "sequence.h"
#include "format.h"
#include "exception.h"


namespace lights {

enum class FileSeekWhence
{
	BEGIN = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END
};

enum class FileBufferingMode
{
	FULL_BUFFERING = _IOFBF, LINE_BUFFERING = _IOLBF, NO_BUFFERING = _IONBF
};


class FileStream
{
public:
	FileStream() = default;

	FileStream(StringView filename, StringView modes)
	{
		this->open(filename, modes);
	}

	~FileStream()
	{
		if (m_file != nullptr)
		{
			this->close();
		}
	}

	/**
	 * @throw Thrown OpenFileError when have error.
	 */
	void open(StringView filename, StringView modes)
	{
		assert(!is_open() && "Cannot open file, becase there is handler that is not close.");
		m_file = std::fopen(filename.data(), modes.data());
		if (m_file == nullptr)
		{
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}

	/**
	 * @throw Thrown OpenFileError when have error.
	 */
	void reopen(StringView filename, StringView modes)
	{
		if (std::freopen(filename.data(), modes.data(), m_file) == nullptr)
		{
			m_file = nullptr;
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}

	bool is_open() const
	{
		return m_file != nullptr;
	}

	std::size_t read(Sequence sequence)
	{
		return std::fread(sequence.data(), sizeof(char), sequence.length(), m_file);
	}

	std::size_t write(SequenceView sequence)
	{
		return std::fwrite(sequence.data(), sizeof(char), sequence.length(), m_file);
	}

	void flush()
	{
		std::fflush(m_file);
	}

	int get_char()
	{
		return std::getc(m_file);
	}

	int put_char(int ch)
	{
		return std::putc(ch, m_file);
	}

	int unget_char(int ch)
	{
		return std::ungetc(ch, m_file);
	}

	int peek()
	{
		int ch = get_char();
		unget_char(ch);
		return ch;
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
		// Return type off_t will fit into suitable type for 32 and 64 architechures.
		return ftello(m_file);
	}

	void seek(std::streamoff off, FileSeekWhence whence)
	{
		// off type off_t will fit into suitable type for 32 and 64 architechures.
		fseeko(m_file, off, static_cast<int>(whence));
	}

	void rewind()
	{
		std::rewind(m_file);
	}

	std::size_t size()
	{
		std::streamoff origin = this->tell();
		this->seek(0, FileSeekWhence::END);
		std::streamoff size = this->tell();
		this->seek(origin, FileSeekWhence::BEGIN);
		return static_cast<std::size_t>(size);
	}

	void close()
	{
		std::fclose(m_file);
		m_file = nullptr;
	}

	void setbuf(char* buffer)
	{
		std::setbuf(m_file, buffer);
	}

	void setvbuf(char* buffer, std::size_t size, FileBufferingMode mode)
	{
		std::setvbuf(m_file, buffer, static_cast<int>(mode), size);
	}

	void write_line(StringView str)
	{
		write(str);
		write(StringView(LIGHTS_LINE_ENDER));
	}

	friend FileStream& stdin_stream();

	friend FileStream& stdout_stream();

	friend FileStream& stderr_stream();

private:
	std::FILE* m_file = nullptr;
};


inline FileStream& stdin_stream()
{
	static FileStream stream;
	stream.m_file = stdin;
	return stream;
}

inline FileStream& stdout_stream()
{
	static FileStream stream;
	stream.m_file = stdout;
	return stream;
}

inline FileStream& stderr_stream()
{
	static FileStream stream;
	stream.m_file = stderr;
	return stream;
}


class FileSinkAdapter: public SinkAdapter
{
public:
	explicit FileSinkAdapter(FileStream& stream):
		m_stream(stream)
	{}

	std::size_t write(SequenceView sequence) override
	{
		return m_stream.write(sequence);
	};

private:
	FileStream& m_stream;
};


inline void dump(const Exception& ex, FileStream& out)
{
	FileSinkAdapter sink(out);
	dump(ex, sink);
}

inline FileStream& operator<< (FileStream& out, const Exception& ex)
{
	dump(ex, out);
	return out;
}

inline FileStream& operator<< (FileStream& out, StringView str)
{
	out.write(str);
	return out;
}

} // namespace lights
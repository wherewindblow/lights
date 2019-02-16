/**
 * file.h
 * @author wherewindblow
 * @date   Jun 17, 2017
 */

#pragma once

#include <cstdio>

#include "config.h"
#include "env.h"
#include "sequence.h"
#include "exception.h"
#include "non_copyable.h"


namespace lights {

enum class FileSeekWhence
{
	BEGIN = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END
};

enum class FileBufferingMode
{
	FULL_BUFFERING = _IOFBF, LINE_BUFFERING = _IOLBF, NO_BUFFERING = _IONBF
};

constexpr const int FILE_DEFAULT_BUFFER_SIZE = BUFSIZ;


/**
 * FileStream provide operation with a file.
 */
class FileStream : public NonCopyable
{
public:
	FileStream() :
		m_std_file(nullptr)
	{}

	/**
	 * Opens a file with @c filename and @c modes
	 * @throw Thrown OpenFileError when have error.
	 */
	FileStream(StringView filename, StringView modes) :
		FileStream()
	{
		open(filename, modes);
	}

	/**
	 * Closes a file automatically when there is a open file.
	 */
	~FileStream()
	{
		if (is_open())
		{
			close();
		}
	}

	/**
	 * Opens a file with @c filename and @c modes
	 * @throw Thrown OpenFileError when have error.
	 */
	void open(StringView filename, StringView modes)
	{
		LIGHTS_ASSERT(!is_open() && "Cannot open file, becase there is handler that is not close.");
		m_std_file = std::fopen(filename.data(), modes.data());
		if (m_std_file == nullptr)
		{
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}

	/**
	 * Reopens a file with @c filename and @c modes
	 * @throw Thrown OpenFileError when have error.
	 */
	void reopen(StringView filename, StringView modes)
	{
		if (std::freopen(filename.data(), modes.data(), m_std_file) == nullptr)
		{
			m_std_file = nullptr;
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}

	/**
	 * Checks the file is open.
	 */
	bool is_open() const
	{
		return m_std_file != nullptr;
	}

	/**
	 * Reads file content into @c sequence.
	 * @return Number of read successfully.
	 */
	std::size_t read(Sequence sequence)
	{
		return std::fread(sequence.data(), sizeof(char), sequence.length(), m_std_file);
	}

	/**
	 * Writes @c sequence into file.
	 * @param sequence
	 * @return Number of write successfully.
	 */
	std::size_t write(SequenceView sequence)
	{
		return std::fwrite(sequence.data(), sizeof(char), sequence.length(), m_std_file);
	}

	/**
	 * Synchronizes with the actual file.
	 */
	void flush()
	{
		std::fflush(m_std_file);
	}

	/**
	 * Reads a character.
	 */
	int get_char()
	{
		return std::getc(m_std_file);
	}

	/**
	 * Writes the character @c ch.
	 */
	int put_char(int ch)
	{
		return std::putc(ch, m_std_file);
	}

	/**
	 * Puts the character @c ch back.
	 */
	int unget_char(int ch)
	{
		return std::ungetc(ch, m_std_file);
	}

	/**
	 * Reads the next character without extracting it.
	 */
	int peek()
	{
		int ch = get_char();
		unget_char(ch);
		return ch;
	}

	/**
	 * Checks is end of file.
	 */
	bool eof()
	{
		return std::feof(m_std_file) != 0;
	}

	/**
	 * Checks have error now.
	 */
	bool error()
	{
		return std::ferror(m_std_file) != 0;
	}

	/**
	 * Clears error state.
	 */
	void clear_error()
	{
		std::clearerr(m_std_file);
	}

	/**
	 * Returns the current file position indicator.
	 */
	std::streamoff tell()
	{
		return env::ftell(m_std_file);
	}

	/**
	 * Moves the file position indicator to a specific location in a file.
	 */
	void seek(std::streamoff off, FileSeekWhence whence)
	{
		env::fseek(m_std_file, off, static_cast<int>(whence));
	}

	/**
	 * Moves the file position indicator to the beginning in a file.
	 */
	void rewind()
	{
		std::rewind(m_std_file);
	}

	/**
	 * Returns the size of a file.
	 */
	std::size_t size()
	{
		std::streamoff origin = this->tell();
		seek(0, FileSeekWhence::END);
		std::streamoff size = this->tell();
		seek(origin, FileSeekWhence::BEGIN);
		return static_cast<std::size_t>(size);
	}

	/**
	 * Closes a file.
	 */
	void close()
	{
		std::fclose(m_std_file);
		m_std_file = nullptr;
	}

	/**
	 * Sets the file buffer.
	 */
	void setbuf(char* buffer)
	{
		std::setbuf(m_std_file, buffer);
	}

	/**
	 * Sets the file buffer with size.
	 */
	void setvbuf(char* buffer, std::size_t size, FileBufferingMode mode)
	{
		std::setvbuf(m_std_file, buffer, static_cast<int>(mode), size);
	}

	/**
	 * Writes @c str and append line end symbol.
	 */
	void write_line(StringView str)
	{
		write(str);
		write(StringView(env::end_line()));
	}

	friend FileStream& stdin_stream();

	friend FileStream& stdout_stream();

	friend FileStream& stderr_stream();

private:
	std::FILE* m_std_file;
};

/**
 * Standard stdin stream.
 */
inline FileStream& stdin_stream()
{
	static FileStream stream;
	stream.m_std_file = stdin;
	return stream;
}

/**
 * Standard stdout stream.
 */
inline FileStream& stdout_stream()
{
	static FileStream stream;
	stream.m_std_file = stdout;
	return stream;
}

/**
 * Standard stderr stream.
 */
inline FileStream& stderr_stream()
{
	static FileStream stream;
	stream.m_std_file = stderr;
	return stream;
}

/**
 * Sink adapter of file stream.
 */
class FileSink: public Sink
{
public:
	explicit FileSink(FileStream& stream):
		m_stream(stream)
	{}

	std::size_t write(SequenceView sequence) override
	{
		return m_stream.write(sequence);
	};

private:
	FileStream& m_stream;
};

/**
 * Dumps a exception @c ex message to file stream @c out.
 */
inline void dump(const Exception& ex, FileStream& out)
{
	FileSink sink(out);
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
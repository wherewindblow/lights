/**
 * file_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <mutex>

#include "../sequence.h"
#include "../file.h"


namespace lights {
namespace sinks {

/**
 * LogMessageWriter ensure every log message is write to backend completely.
 * And support difference policy to flush log message to backend.
 */
class LogMessageWriter
{
public:
	/**
	 * Creates writer.
	 */
	LogMessageWriter(FileStream* file = nullptr) :
		m_file(file),
		m_buffer_length(0),
		m_last_flush_time(0)
	{}

	/**
	 * Sets log message write target.
	 */
	void set_write_target(FileStream* file)
	{
		m_file = file;
		m_buffer_length = 0;
	}

	/**
	 * Writes log message into backend.
	 */
	std::size_t write(SequenceView log_msg)
	{
		std::size_t len = m_file->write(log_msg);
		m_buffer_length += len;

		if (m_buffer_length > FILE_DEFAULT_BUFFER_SIZE)
		{
			m_buffer_length -= FILE_DEFAULT_BUFFER_SIZE;
			m_last_flush_time = std::time(nullptr);
		}

		return len;
	}

	/**
	 * Flushes underlying buffer if it's been long time no flush.
	 * Use to ensure log message to be write to file.
	 */
	void flush_by_timeout(std::time_t timeout)
	{
		std::time_t cur_time = std::time(nullptr);
		if (m_last_flush_time - cur_time >= timeout)
		{
			m_file->flush();
			m_last_flush_time = cur_time;
			m_buffer_length = 0;
		}
	}

private:
	FileStream* m_file;
	std::size_t m_buffer_length;
	std::time_t m_last_flush_time;
};


/**
 * SimpleFileSink write all log message into one file.
 */
class SimpleFileSink: public Sink
{
public:
	/**
	 * Creates sink.
	 */
	SimpleFileSink(StringView filename);

	/**
	 * Writes log message into backend.
	 * @details Write is atomic.
	 */
	std::size_t write(SequenceView log_msg) override;

	/**
	 * Flushes underlying buffer if it's been long time no flush.
	 * Use to ensure log message to be write to file.
	 */
	void flush_by_timeout(std::time_t timeout)
	{
		m_msg_writer.flush_by_timeout(timeout);
	}

private:
	FileStream m_file;
	LogMessageWriter m_msg_writer;
	std::mutex m_mutex;
};


/**
 * SizeRotatingFileSink write log message into file. If file is achieve size limit will rotate to next file.
 */
class SizeRotatingFileSink: public Sink
{
public:
	SizeRotatingFileSink();

	/**
	 * Init file name format.
	 * @param name_format  Format string that use "{}" as a placeholder.
	 * @note This init is necessary.
	 */
	void init_name_format(const std::string& name_format);

	/**
	 * @note If not init max size, the biggest of std::size_t value is use.
	 */
	void init_max_size(std::size_t max_size);

	/**
	 * @note If not int max files, it'll not cycle to use file name.
	 */
	void init_max_files(std::size_t max_files);

	/**
	 * Ends initialization.
	 */
	void end_init();

	/**
	 * Writes log message into backend.
	 * @details Write is atomic.
	 */
	std::size_t write(SequenceView log_msg) override;

	/**
	 * Flushes underlying buffer if it's been long time no flush.
	 * Use to ensure log message to be write to file.
	 */
	void flush_by_timeout(std::time_t timeout)
	{
		m_msg_writer.flush_by_timeout(timeout);
	}

private:
	void fill_remain();

	void rotate(std::size_t expect_size);

	bool can_init;
	std::string m_name_format;
	std::size_t m_max_size;
	std::size_t m_max_files;
	FileStream m_file;
	LogMessageWriter m_msg_writer;
	std::size_t m_index;
	std::size_t m_current_size;
	std::mutex m_mutex;
};


/**
 * TimeRotatingFileSink write log message into file. If time is achieve limit will rotate to next file.
 */
class TimeRotatingFileSink: public Sink
{
public:
	static constexpr std::time_t ONE_DAY_SECONDS = 3600 * 24;

	/**
	 * @param name_format  Must have a placeholder "{}" and it'll replace by rotating time.
	 * @param duration     Within a duration, all message will log to same file.
	 * @param day_point    The seconds of a day that will execute rotating.
	 */
	TimeRotatingFileSink(std::string name_format,
						 std::time_t duration = ONE_DAY_SECONDS,
						 std::time_t day_point = 0);

	/**
	 * Writes log message into backend.
	 * @details Write is atomic.
	 */
	std::size_t write(SequenceView log_msg) override;

	/**
	 * Flushes underlying buffer if it's been long time no flush.
	 * Use to ensure log message to be write to file.
	 */
	void flush_by_timeout(std::time_t timeout)
	{
		m_msg_writer.flush_by_timeout(timeout);
	}

private:
	void rotate();

	std::string m_name_format;
	std::time_t m_duration;
	std::time_t m_day_point;
	std::time_t m_next_rotating_time;
	FileStream m_file;
	LogMessageWriter m_msg_writer;
	std::mutex m_mutex;
};

} // namespace sinks
} // namespace lights

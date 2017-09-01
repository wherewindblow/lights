/**
 * bm_format.cpp
 * @author wherewindblow
 * @date   Aug 23, 2017
 */

#include "bm_format.h"

#include <sstream>

#include <benchmark/benchmark.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <lights/format.h>
#include <lights/ostream.h>
#include <lights/sink_adapter.h>


#define BUFFER_SIZE 500
#define FORMAT_INTEGER 1234
#define FORMAT_FLOAT 56.78
#define FORMAT_STRING "910111"


// ------------------- Format integer. -----------------------

void BM_format_int_std_sprintf(benchmark::State& state)
{
	char buf[BUFFER_SIZE];
	while (state.KeepRunning())
	{
		sprintf(buf, "%d", FORMAT_INTEGER);
	}
}

void BM_format_int_std_sstream(benchmark::State& state)
{
	std::ostringstream stream;
	while (state.KeepRunning())
	{
		stream.str("");
		stream << FORMAT_INTEGER;
		stream.str();
	}
}

void BM_format_int_fmt_MemoryWriter_write(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_fmt_MemoryWriter_insert(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_INTEGER;
		writer.c_str();
	}
}

void BM_format_int_lights_TextWriter_write(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_lights_TextWriter_insert(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_INTEGER;
		writer.c_str();
	}
}


void BM_format_int_not_reuse_std_sprintf(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		char buf[BUFFER_SIZE];
		sprintf(buf, "%d", FORMAT_INTEGER);
	}
}

void BM_format_int_not_reuse_std_sstream(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		std::ostringstream stream;
		stream << FORMAT_INTEGER;
		stream.str();
	}
}

void BM_format_int_not_reuse_fmt_MemoryWriter(benchmark::State& state)
{

	while (state.KeepRunning())
	{
		fmt::MemoryWriter writer;
		writer.write("{}", FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_not_reuse_lights_TextWriter_write(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		lights::TextWriter<> writer;
		writer.write("{}", FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_not_reuse_lights_TextWriter_insert(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		lights::TextWriter<> writer;
		writer << FORMAT_INTEGER;
		writer.c_str();
	}
}

void BM_format_int_fmt_FormatInt(benchmark::State& state)
{
	fmt::FormatInt formater(FORMAT_INTEGER);
	while (state.KeepRunning())
	{
		formater.c_str();
	}
}

void BM_format_int_lights_IntegerFormater(benchmark::State& state)
{
	lights::details::IntegerFormater formater;
	while (state.KeepRunning())
	{
		formater.format(FORMAT_INTEGER);
	}
}

void BM_format_int_not_reuse_fmt_FormatInt(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		fmt::FormatInt formater(FORMAT_INTEGER);
		formater.c_str();
	}
}

void BM_format_int_not_reuse_lights_IntegerFormater(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		lights::details::IntegerFormater formater;
		formater.format(FORMAT_INTEGER);
	}
}


// ------------------- Format integer with binary. -----------------------

void BM_format_int_binary_fmt_MemoryWriter(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << fmt::bin(FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_binary_lights_TextWriter(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", lights::binary(FORMAT_INTEGER));
		writer.c_str();
	}
}


// ------------------- Format integer with octal. -----------------------

void BM_format_int_octal_std_sprintf(benchmark::State& state)
{
	char buf[BUFFER_SIZE];
	while (state.KeepRunning())
	{
		sprintf(buf, "%o", FORMAT_INTEGER);
	}
}

void BM_format_int_octal_fmt_MemoryWriter(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << fmt::oct(FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_octal_lights_TextWriter(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", lights::octal(FORMAT_INTEGER));
		writer.c_str();
	}
}


// ------------------- Format integer with hex. -----------------------

void BM_format_int_hex_std_sprintf(benchmark::State& state)
{
	char buf[BUFFER_SIZE];
	while (state.KeepRunning())
	{
		sprintf(buf, "%x", FORMAT_INTEGER);
	}
}

void BM_format_int_hex_fmt_MemoryWriter(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << fmt::hex(FORMAT_INTEGER);
		writer.c_str();
	}
}

void BM_format_int_hex_lights_TextWriter(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", lights::hex_lower_case(FORMAT_INTEGER));
		writer.c_str();
	}
}


// ------------------- Format float. -----------------------

void BM_format_float_std_sprintf(benchmark::State& state)
{
	char buf[BUFFER_SIZE];
	while (state.KeepRunning())
	{
		sprintf(buf, "%f", FORMAT_FLOAT);
	}
}

void BM_format_float_std_sstream(benchmark::State& state)
{
	std::ostringstream stream;
	while (state.KeepRunning())
	{
		stream.str("");
		stream << FORMAT_FLOAT;
		stream.str();
	}
}

void BM_format_float_fmt_MemoryWriter_write(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", FORMAT_FLOAT);
		writer.c_str();
	}
}

void BM_format_float_fmt_MemoryWriter_insert(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_FLOAT;
		writer.c_str();
	}
}

void BM_format_float_lights_TextWriter_write(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", FORMAT_FLOAT);
		writer.c_str();
	}
}

void BM_format_float_lights_TextWriter_insert(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_FLOAT;
		writer.c_str();
	}
}


// ------------------- Format string. -----------------------

void BM_format_string_std_sprintf(benchmark::State& state)
{
	char buf[BUFFER_SIZE];
	while (state.KeepRunning())
	{
		sprintf(buf, "%s", FORMAT_STRING);
	}
}

void BM_format_string_std_sstream(benchmark::State& state)
{
	std::ostringstream stream;
	while (state.KeepRunning())
	{
		stream.str("");
		stream << FORMAT_STRING;
		stream.str();
	}
}

void BM_format_string_fmt_MemoryWriter_write(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", FORMAT_STRING);
		writer.c_str();
	}
}

void BM_format_string_fmt_MemoryWriter_insert(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_STRING;
		writer.c_str();
	}
}

void BM_format_string_lights_TextWriter_write(benchmark::State& state)
{
	lights::TextWriter<> writer;
	// An optimize way to avoid calculate the lenght of string again and again.
//	lights::StringView str = FORMAT_STRING;
	while (state.KeepRunning())
	{
		writer.clear();
//		writer.write("{}", str); // If put literal string will increase some CPU use.
		writer.write("{}", FORMAT_STRING);
		writer.c_str();
	}
}

void BM_format_string_lights_TextWriter_insert(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_STRING;
		writer.c_str();
	}
}


// ----------------------- Format mix. --------------------------

void BM_format_mix_std_sprintf(benchmark::State& state)
{
	char buf[BUFFER_SIZE];
	while (state.KeepRunning())
	{
		sprintf(buf, "%d%f%s", FORMAT_INTEGER, FORMAT_FLOAT, FORMAT_STRING);
	}
}

void BM_format_mix_std_sstream(benchmark::State& state)
{
	std::ostringstream stream;
	while (state.KeepRunning())
	{
		stream.str("");
		stream << FORMAT_INTEGER << FORMAT_FLOAT << FORMAT_STRING;
		stream.str();
	}
}

void BM_format_mix_fmt_MemoryWriter_write(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}{}{}", FORMAT_INTEGER, FORMAT_FLOAT, FORMAT_STRING);
		writer.c_str();
	}
}

void BM_format_mix_fmt_MemoryWriter_insert(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_INTEGER << FORMAT_FLOAT << FORMAT_STRING;
		writer.c_str();
	}
}

void BM_format_mix_lights_TextWriter_write(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}{}{}", FORMAT_INTEGER, FORMAT_FLOAT, FORMAT_STRING);
		writer.c_str();
	}
}

void BM_format_mix_lights_TextWriter_insert(benchmark::State& state)
{
	lights::TextWriter<> writer;
	while (state.KeepRunning())
	{
		writer.clear();
		writer << FORMAT_INTEGER << FORMAT_FLOAT << FORMAT_STRING;
		writer.c_str();
	}
}


// ----------------------- Format time. --------------------------

void BM_format_time_std_strftime(benchmark::State& state)
{
	std::time_t time = std::time(nullptr);

	while (state.KeepRunning())
	{
		std::tm tm;
		localtime_r(&time, &tm);
		char buf[20];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
	}
}

void BM_format_time_fmt_MemoryWriter_write(benchmark::State& state)
{
	std::time_t time = std::time(nullptr);
	fmt::MemoryWriter writer;

	while (state.KeepRunning())
	{
		std::tm tm;
		localtime_r(&time, &tm);

		writer.clear();
		writer.write("[{:d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
			  tm.tm_year + 1900,
			  tm.tm_mon + 1,
			  tm.tm_mday,
			  tm.tm_hour,
			  tm.tm_min,
			  tm.tm_sec);
	}
}

void BM_format_time_fmt_MemoryWriter_insert(benchmark::State& state)
{
	std::time_t time = std::time(nullptr);
	fmt::MemoryWriter writer;

	while (state.KeepRunning())
	{
		std::tm tm;
		localtime_r(&time, &tm);

		writer.clear();
		writer << static_cast<unsigned int>(tm.tm_year + 1900) << '-'
			   << fmt::pad(static_cast<unsigned int>(tm.tm_mon + 1), 2, '0') << '-'
			   << fmt::pad(static_cast<unsigned int>(tm.tm_mday), 2, '0') << ' '
			   << fmt::pad(static_cast<unsigned int>(tm.tm_hour), 2, '0') << ':'
			   << fmt::pad(static_cast<unsigned int>(tm.tm_min), 2, '0') << ':'
			   << fmt::pad(static_cast<unsigned int>(tm.tm_sec), 2, '0');
	}
}

void BM_format_time_lights_TextWriter_write(benchmark::State& state)
{
	std::time_t time = std::time(nullptr);
	lights::TextWriter<> writer;

	while (state.KeepRunning())
	{
		std::tm tm;
		localtime_r(&time, &tm);

		writer.clear();
		writer.write("{}-{}-{} {}:{}:{}",
					 tm.tm_year + 1900,
					 lights::pad(static_cast<unsigned int>(tm.tm_mon + 1), '0', 2),
					 lights::pad(static_cast<unsigned int>(tm.tm_mday), '0', 2),
					 lights::pad(static_cast<unsigned int>(tm.tm_hour), '0', 2),
					 lights::pad(static_cast<unsigned int>(tm.tm_min), '0', 2),
					 lights::pad(static_cast<unsigned int>(tm.tm_sec), '0', 2));
	}
}

void BM_format_time_lights_TextWriter_insert(benchmark::State& state)
{
	std::time_t time = std::time(nullptr);
	lights::TextWriter<> writer;

	while (state.KeepRunning())
	{
		std::tm tm;
		localtime_r(&time, &tm);

		writer.clear();
		writer << tm.tm_year + 1900 << '-'
			<< lights::pad(static_cast<unsigned int>(tm.tm_mon + 1), '0', 2)
			<< lights::pad(static_cast<unsigned int>(tm.tm_mday), '0', 2)
			<< lights::pad(static_cast<unsigned int>(tm.tm_hour), '0', 2)
			<< lights::pad(static_cast<unsigned int>(tm.tm_min), '0', 2)
			<< lights::pad(static_cast<unsigned int>(tm.tm_sec), '0', 2);
	}
}

void BM_format_time_lights_to_string(benchmark::State& state)
{
	lights::TextWriter<> writer;
	auto timestamp = lights::current_timestamp();

	while (state.KeepRunning())
	{
		writer.clear();
		lights::to_string(lights::make_format_sink_adapter(writer), timestamp);
	}
}


// ----------------------- Format user-defined type. --------------------------

struct Coordinate
{
	Coordinate() = default;
	int coordinate_x = 1, coordinate_y = 2;
};


inline std::ostream& operator<<(std::ostream& ostream, const Coordinate& coordinate)
{
	ostream << coordinate.coordinate_x << ':' << coordinate.coordinate_y;
	return ostream;
}


template <typename Sink>
inline void append(lights::FormatSinkAdapter<Sink> out, const Coordinate& coordinate)
{
	using lights::operator<<;
	out << coordinate.coordinate_x << ':' << coordinate.coordinate_y;
}


struct CoordinateEx
{
	CoordinateEx() = default;
	Coordinate coordinate;
	int coordinate_z = 3;
};


template <typename Sink>
void append(lights::FormatSinkAdapter<Sink> out, const CoordinateEx& coordinate_ex)
{
	using lights::operator<<;
	out << coordinate_ex.coordinate << ':' << coordinate_ex.coordinate_z;
}


void BM_format_custom_std_sstream(benchmark::State& state)
{
	std::ostringstream stream;
	Coordinate coordinate;

	while (state.KeepRunning())
	{
		stream.str("");
		stream << coordinate;
	}
}

void BM_format_custom_fmt_MemoryWriter(benchmark::State& state)
{
	fmt::MemoryWriter writer;
	Coordinate coordinate;

	while (state.KeepRunning())
	{
		writer.clear();
		writer.write("{}", coordinate);
	}
}

/**
 * Use `void append(lights::FormatSinkAdapter<Sink> out, const Coordinate& coordinate)`
 * is more faster than `std::ostream& operator<<(std::ostream& ostream, const Coordinate& coordinate)`
 */
void BM_format_custom_lights_TextWriter(benchmark::State& state)
{
	lights::TextWriter<> writer;
	Coordinate coordinate;

	while (state.KeepRunning())
	{
		writer.clear();
		writer << coordinate;
	}
}


// ----------------------- Format implementation way. --------------------------

namespace lights {

template <>
class FormatSinkAdapter<char*>
{
public:
	explicit FormatSinkAdapter(char* sink, std::size_t max_size) :
		m_sink(sink), m_max_size(max_size) {}

	void append(char ch)
	{
		if (m_current_size + 1 <= m_max_size)
		{
			m_sink[m_current_size] = ch;
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
			lights::copy_array(m_sink + m_current_size, str.data(), str.length());
			m_current_size += str.length();
		}
	}

	void reset()
	{
		m_current_size = 0;
	}

private:
	char* m_sink;
	std::size_t m_max_size;
	std::size_t m_current_size = 0;
};

} // namespace lights


void BM_format_lights_insert_template(benchmark::State& state)
{
	char buf[lights::WRITER_BUFFER_SIZE_DEFAULT];
	lights::zero_array(buf);
	// Why not use TextWriter directly, because TextWriter have itself own
	// optimization way of insert integer. And may not fair for using virtual
	// function implementation version.
	lights::FormatSinkAdapter<char*> format_sink_adapter(buf, lights::size_of_array(buf));

	while (state.KeepRunning())
	{
		format_sink_adapter.reset();
		format_sink_adapter << FORMAT_INTEGER;
	}
}


class CharArrayAdapter: public lights::SinkAdapter
{
public:
	CharArrayAdapter(char* sink, std::size_t max_size) :
		m_sink(sink), m_max_size(max_size) {}

	virtual std::size_t write(lights::SequenceView buffer)
	{
		lights::StringView str = lights::to_string_view(buffer);
		if (m_current_size + str.length() <= m_max_size)
		{
			lights::copy_array(m_sink + m_current_size, str.data(), str.length());
			m_current_size += str.length();
			return buffer.length();
		}
		return 0;
	}

	void reset()
	{
		m_current_size = 0;
	}

private:
	char* m_sink;
	std::size_t m_max_size;
	std::size_t m_current_size = 0;
};


void BM_format_lights_insert_virtual(benchmark::State& state)
{
	char buf[lights::WRITER_BUFFER_SIZE_DEFAULT];
	lights::zero_array(buf);
	CharArrayAdapter array_adapter(buf, lights::size_of_array(buf));
	lights::SinkAdapter& adapter = array_adapter;

	while (state.KeepRunning())
	{
		array_adapter.reset();
		lights::make_format_sink_adapter(adapter) << FORMAT_INTEGER;
	}
}


void BM_format()
{
	BENCHMARK(BM_format_int_std_sprintf);
	BENCHMARK(BM_format_int_std_sstream);
	BENCHMARK(BM_format_int_fmt_MemoryWriter_write);
	BENCHMARK(BM_format_int_fmt_MemoryWriter_insert);
	BENCHMARK(BM_format_int_lights_TextWriter_write);
	BENCHMARK(BM_format_int_lights_TextWriter_insert);

	BENCHMARK(BM_format_int_not_reuse_std_sprintf);
	BENCHMARK(BM_format_int_not_reuse_std_sstream);
	BENCHMARK(BM_format_int_not_reuse_fmt_MemoryWriter);
	BENCHMARK(BM_format_int_not_reuse_lights_TextWriter_write);
	BENCHMARK(BM_format_int_not_reuse_lights_TextWriter_insert);

	BENCHMARK(BM_format_int_fmt_FormatInt);
	BENCHMARK(BM_format_int_lights_IntegerFormater);
	BENCHMARK(BM_format_int_not_reuse_fmt_FormatInt);
	BENCHMARK(BM_format_int_not_reuse_lights_IntegerFormater);

	BENCHMARK(BM_format_int_binary_fmt_MemoryWriter);
	BENCHMARK(BM_format_int_binary_lights_TextWriter);

	BENCHMARK(BM_format_int_octal_std_sprintf);
	BENCHMARK(BM_format_int_octal_fmt_MemoryWriter);
	BENCHMARK(BM_format_int_octal_lights_TextWriter);

	BENCHMARK(BM_format_int_hex_std_sprintf);
	BENCHMARK(BM_format_int_hex_fmt_MemoryWriter);
	BENCHMARK(BM_format_int_hex_lights_TextWriter);

	BENCHMARK(BM_format_float_std_sprintf);
	BENCHMARK(BM_format_float_std_sstream);
	BENCHMARK(BM_format_float_fmt_MemoryWriter_write);
	BENCHMARK(BM_format_float_fmt_MemoryWriter_insert);
	BENCHMARK(BM_format_float_lights_TextWriter_write);
	BENCHMARK(BM_format_float_lights_TextWriter_insert);

	BENCHMARK(BM_format_string_std_sprintf);
	BENCHMARK(BM_format_string_std_sstream);
	BENCHMARK(BM_format_string_fmt_MemoryWriter_write);
	BENCHMARK(BM_format_string_fmt_MemoryWriter_insert);
	BENCHMARK(BM_format_string_lights_TextWriter_write);
	BENCHMARK(BM_format_string_lights_TextWriter_insert);

	BENCHMARK(BM_format_mix_std_sprintf);
	BENCHMARK(BM_format_mix_std_sstream);
	BENCHMARK(BM_format_mix_fmt_MemoryWriter_write);
	BENCHMARK(BM_format_mix_fmt_MemoryWriter_insert);
	BENCHMARK(BM_format_mix_lights_TextWriter_write);
	BENCHMARK(BM_format_mix_lights_TextWriter_insert);

	BENCHMARK(BM_format_time_std_strftime);
	BENCHMARK(BM_format_time_fmt_MemoryWriter_write);
	BENCHMARK(BM_format_time_fmt_MemoryWriter_insert);
	BENCHMARK(BM_format_time_lights_TextWriter_write);
	BENCHMARK(BM_format_time_lights_TextWriter_insert);
	BENCHMARK(BM_format_time_lights_to_string);

	BENCHMARK(BM_format_custom_std_sstream);
	BENCHMARK(BM_format_custom_fmt_MemoryWriter);
	BENCHMARK(BM_format_custom_lights_TextWriter);

	BENCHMARK(BM_format_lights_insert_template);
	BENCHMARK(BM_format_lights_insert_virtual);
}

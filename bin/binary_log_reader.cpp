/**
 * binary_log_reader.cpp
 * @author wherewindblow
 * @date   Aug 19, 2017
 */

#include <string>

#include <lights/format.h>
#include <lights/logger.h>


void read_log(lights::StringView log_filename, lights::StringView str_table_filename, std::streamoff line)
{
	auto str_table_ptr = lights::StringTable::create(str_table_filename);

	lights::BinaryLogReader reader(log_filename, str_table_ptr);
	reader.jump(line);

	while (!reader.eof())
	{
		lights::StringView log = reader.read();
		if (!lights::is_valid(log))
		{
			break;
		}
		lights::stdout_stream().write_line(log);
	}
}


int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		lights::stdout_stream() << "Pass a binary log file and read it.";
		lights::stdout_stream() << "    %1: binary log";
		lights::stdout_stream() << "    %2: log string table or default is 'log_str_table'";
		lights::stdout_stream() << "    %3: read at line";
		return EXIT_FAILURE;
	}

	const char* log_filename = argv[1];
	const char* str_table_filename = (argc > 2) ? argv[2] : "log_str_table";
	const std::streamoff line = (argc > 3) ? std::stoll(argv[3]) : 0;

	try
	{
		read_log(log_filename, str_table_filename, line);
	}
	catch (lights::Exception& ex)
	{
		lights::stdout_stream() << ex;
	}

	return EXIT_SUCCESS;
}
/**
 * binary_log_reader.cpp
 * @author wherewindblow
 * @date   Aug 19, 2017
 */

#include <string>
#include <thread>

#include <lights/format.h>
#include <lights/logger.h>


enum class ReadModeType
{
	JUMP_TO_LINE,
	FOLLOW_FILE_GROWS,
};


void read_log(lights::StringView log_filename, lights::StringView str_table_filename, ReadModeType read_mode, std::streamoff line)
{
	auto str_table_ptr = lights::StringTable::create(str_table_filename);

	lights::BinaryLogReader reader(log_filename, str_table_ptr);
	if (read_mode == ReadModeType::JUMP_TO_LINE)
	{
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
	else
	{
		reader.jump_to_end();
		while (true)
		{
			if (reader.have_new_message())
			{
				reader.clear_eof();
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
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
	}
}


int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		lights::stdout_stream() << "Pass a binary log file and read it.\n";
		lights::stdout_stream() << "    %1: Binary log filename.\n";
		lights::stdout_stream() << "    %2: Log string table filename or default is 'log_str_table'.\n";
		lights::stdout_stream() << "    %3: Mode: jump to line('j') or follow file grows('f').\n";
		lights::stdout_stream() << "    %4: Read at line when mode is jump to line('j').\n";
		return EXIT_FAILURE;
	}

	const char* log_filename = argv[1];
	const char* str_table_filename = (argc > 2) ? argv[2] : "log_str_table";
	ReadModeType read_mode = ReadModeType::JUMP_TO_LINE;
	if (argc > 3 && *argv[3] == 'f')
	{
		read_mode = ReadModeType::FOLLOW_FILE_GROWS;
	}
	const std::streamoff line = (argc > 4) ? std::stoll(argv[4]) : 0;

	try
	{
		read_log(log_filename, str_table_filename, read_mode, line);
	}
	catch (lights::Exception& ex)
	{
		lights::stdout_stream() << ex;
	}

	return EXIT_SUCCESS;
}
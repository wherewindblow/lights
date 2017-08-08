/**
 * exception.cpp
 * @author wherewindblow
 * @date   Jul 23, 2017
 */

#include "exception.h"


namespace lights {

StringView LightsErrorCodeCategory::name() const
{
	return "LightsErrorCodeCategory";
}


ErrorCodeDescriptions LightsErrorCodeCategory::descriptions(int code) const
{
	static ErrorCodeDescriptions map[] = {
		{"Success"},
		{"Invalid argument", "Invalid argument: {}"},
		{"Open file failure", "Open file \"{}\" failure: {}"}
	};

	if (is_safe_index(code, map))
	{
		return map[static_cast<std::size_t>(code)];
	}
	else
	{
		static ErrorCodeDescriptions unknow = {"Unknow error"};
		return unknow;
	}
}


const char* Exception::what() const noexcept
{
	return m_code_category.descriptions(m_code).without_args.data();
}


void Exception::dump_message(SinkAdapter& out) const
{
	StringView view = code_category().descriptions(m_code).without_args;
	out.write(view);
}


void OpenFileError::dump_message(SinkAdapter& out) const
{
	write(make_format_sink_adapter(out),
		  code_category().descriptions(code()).with_args,
		  m_filename,
		  current_error());
}

void InvalidArgument::dump_message(SinkAdapter& out) const
{
	write(make_format_sink_adapter(out),
		  code_category().descriptions(code()).with_args,
		  m_description);
}


void dump(const Exception& ex, SinkAdapter& out)
{
	ex.dump_message(out);
	out << " <-- ";
	auto& loc = ex.occur_location();
	out << loc.file() << ":";
	to_string(make_format_sink_adapter(out), loc.line());
	out << "#" << loc.function();
}

} // namespace lights
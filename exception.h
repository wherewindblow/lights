/**
 * exception.h
 * @author wherewindblow
 * @date   Jul 02, 2017
 */

#pragma once

#include <exception>

#include "sink.h"
#include "format.h"
#include "current_function.hpp"
#include "common_function.h"


namespace lights {

class SourceLocation
{
public:
	SourceLocation() = default;
	SourceLocation(const char* file, std::uint32_t line, const char* function):
		m_file(file), m_function(function), m_line(line) {}

	const char* file() const
	{
		return m_file;
	}

	const char* function() const
	{
		return m_function;
	}

	std::uint32_t line() const
	{
		return m_line;
	}

private:
	const char* m_file;
	const char* m_function;
	std::uint32_t m_line;
};

#define LIGHTS_CURRENT_SOURCE_LOCATION lights::SourceLocation(__FILE__, __LINE__, BOOST_CURRENT_FUNCTION)


namespace error_code {

enum Type
{
	SUCCESS,
	INVALID_ARGUMENT,
	OPEN_FILE_FAILURE,
};

} // namespace error_code


struct ErrorCodeDescriptions
{
	ErrorCodeDescriptions(StringView without_args, StringView with_args) :
		without_args(without_args), with_args(with_args)
	{}

	ErrorCodeDescriptions(StringView without_args) :
		ErrorCodeDescriptions(without_args, without_args)
	{}

	StringView without_args;
	StringView with_args;
};


inline const ErrorCodeDescriptions& to_descriptions(int code)
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

class Exception: public std::exception
{
public:
	Exception(const SourceLocation& occur_location, int code):
		m_occur_location(occur_location), m_code(code)
	{
	}

	const SourceLocation& occur_location() const
	{
		return m_occur_location;
	}

	const char* what() const noexcept override
	{
		return to_descriptions(m_code).without_args.data;
	}

	int code() const
	{
		return m_code;
	}

	virtual void dump_message(SinkAdapter& out) const
	{
		StringView view = to_descriptions(m_code).without_args;
		out.write(view.data, view.length);
	}

private:
	SourceLocation m_occur_location;
	int m_code;
};


class OpenFileError: public Exception
{
public:
	OpenFileError(const SourceLocation& occur_location, StringView filename):
		Exception(occur_location, error_code::OPEN_FILE_FAILURE), m_filename(filename.to_string())
	{
	}

	void dump_message(SinkAdapter& out) const override
	{
		write(make_format_sink_adapter(out), to_descriptions(code()).with_args, m_filename, current_error());
	}

private:
	std::string m_filename;
};


class InvalidArgument: public Exception
{
public:
	InvalidArgument(const SourceLocation& occur_location, StringView description):
		Exception(occur_location, error_code::INVALID_ARGUMENT), m_description(description.to_string())
	{
	}

	void dump_message(SinkAdapter& out) const override
	{
		write(make_format_sink_adapter(out), to_descriptions(code()).with_args, m_description);
	}

private:
	std::string m_description;
};


/**
 * @arg ... Expand to fmt and arg ...
 * @note Expand macro method is not standard.
 */
#define LIGHTS_THROW_EXCEPTION(ExceptionType, ...) \
        throw ExceptionType(LIGHTS_CURRENT_SOURCE_LOCATION, ##__VA_ARGS__);

} // namespace lights

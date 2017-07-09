/**
 * exception.h
 * @author wherewindblow
 * @date   Jul 02, 2017
 */

#pragma once

#include <exception>

#include "format.h"


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

	uint32_t line() const
	{
		return m_line;
	}

private:
	const char* m_file;
	const char* m_function;
	std::uint32_t m_line;
};

#define LIGHTS_CURRENT_SOURCE_LOCATION lights::SourceLocation(__FILE__, __LINE__, __func__)


class Exception: public std::exception
{
public:
	Exception(const SourceLocation& occur_location, StringView what):
		m_occur_location(occur_location), m_what(what.to_string())
	{
	}

	virtual ~Exception()
	{
	}

	const SourceLocation& occur_location() const
	{
		return m_occur_location;
	}

	const char* what() const noexcept override
	{
		return m_what.c_str();
	}

private:
	SourceLocation m_occur_location;
	std::string m_what;
};


class RuntimeError: public Exception
{
public:
	RuntimeError(const SourceLocation& occur_location, StringView what):
		Exception(occur_location, what)
	{
	}
};

class OpenFileError: public RuntimeError
{
public:
	OpenFileError(const SourceLocation& occur_location, StringView what):
		RuntimeError(occur_location, what)
	{
	}
};

class LogicError: public Exception
{
public:
	LogicError(const SourceLocation& occur_location, StringView what):
		Exception(occur_location, what)
	{
	}
};

class InvalidArgument: public LogicError
{
public:
	InvalidArgument(const SourceLocation& occur_location, StringView what):
		LogicError(occur_location, what)
	{
	}
};


/**
 * @arg ... Expand to fmt and arg ...
 */
#define LIGHTS_THROW_EXCEPTION(ExceptionType, ...) \
    { \
        lights::TextWriter<> writer; \
        writer.write(__VA_ARGS__);   \
        throw ExceptionType(LIGHTS_CURRENT_SOURCE_LOCATION, writer.c_str()); \
    }

} // namespace lights

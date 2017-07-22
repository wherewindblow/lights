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


class ErrorCodeCategory
{
public:
	ErrorCodeCategory() = default;
	~ErrorCodeCategory() = default;

	virtual StringView name() const = 0;
	virtual ErrorCodeDescriptions descriptions(int code) const = 0;
};


class LightsErrorCodeCategory: public ErrorCodeCategory
{
public:
	virtual StringView name() const
	{
		return "LightsErrorCodeCategory";
	}

	virtual ErrorCodeDescriptions descriptions(int code) const
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

	static LightsErrorCodeCategory& instance()
	{
		static LightsErrorCodeCategory category;
		return category;
	}
};

class Exception: public std::exception
{
public:
	Exception(const SourceLocation& occur_location, int code, const ErrorCodeCategory& code_category = LightsErrorCodeCategory::instance()):
		m_occur_location(occur_location), m_code(code), m_code_category(code_category)
	{
	}

	const SourceLocation& occur_location() const
	{
		return m_occur_location;
	}

	const char* what() const noexcept override
	{
		return m_code_category.descriptions(m_code).without_args.data;
	}

	int code() const
	{
		return m_code;
	}

	const ErrorCodeCategory& code_category() const
	{
		return m_code_category;
	}

	virtual void dump_message(SinkAdapter& out) const
	{
		StringView view = code_category().descriptions(m_code).without_args;
		out.write(view.data, view.length);
	}

private:
	SourceLocation m_occur_location;
	int m_code;
	const ErrorCodeCategory& m_code_category;
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
		write(make_format_sink_adapter(out),
			  code_category().descriptions(code()).with_args,
			  m_filename,
			  current_error());
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
		write(make_format_sink_adapter(out),
			  code_category().descriptions(code()).with_args,
			  m_description);
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


inline void dump(const Exception& ex, SinkAdapter& out)
{
	ex.dump_message(out);
	out << " <-- ";
	auto& loc = ex.occur_location();
	out << loc.file() << ":";
	to_string(make_format_sink_adapter(out), loc.line());
	out << "#" << loc.function();
}

inline SinkAdapter& operator<< (SinkAdapter& out, const Exception& ex)
{
	dump(ex, out);
	return out;
}


template <typename Sink>
class FormatSelfSinkAdapter: public SinkAdapter
{
public:
	FormatSelfSinkAdapter(FormatSinkAdapter<Sink> out):
		m_out(out)
	{}

	std::size_t write(const void* buf, std::size_t len) override
	{
		m_out.append(StringView{static_cast<const char*>(buf), len});
		return len;
	};
private:
	FormatSinkAdapter<Sink> m_out;
};


template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, const Exception& ex)
{
	FormatSelfSinkAdapter<Sink> adapter(out);
	adapter << ex;
}

} // namespace lights

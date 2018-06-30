/**
 * exception.h
 * @author wherewindblow
 * @date   Jul 02, 2017
 */

#pragma once

#include <cassert>
#include <exception>

#include "config.h"
#include "sequence.h"
#include "sink_adapter.h"
#include "current_function.hpp"
#include "common.h"


namespace lights {

/**
 * SourceLocation records the source file location.
 */
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

/**
 * Returns the current source file location.
 */
#define LIGHTS_CURRENT_SOURCE_LOCATION lights::SourceLocation(__FILE__, __LINE__, BOOST_CURRENT_FUNCTION)

/**
 * Returns invalid source location.
 */
inline SourceLocation invalid_source_location()
{
	return { nullptr, 0, nullptr };
}

/**
 * Checks source location is valid.
 */
inline bool is_valid(const SourceLocation& location)
{
	return location.file() != nullptr && location.function() != nullptr;
}

namespace error_code {

enum Type
{
	SUCCESS,
	ASSERTION_ERROR,
	INVALID_ARGUMENT,
	OPEN_FILE_FAILURE,
};

} // namespace error_code


/**
 * ErrorCodeDescriptions have without arguments description and with arguments description.
 * To get full descrition must use @c with_args and format with arguments.
 */
class ErrorCodeDescriptions
{
public:
	enum DescriptionType
	{
		TYPE_WITHOUT_ARGS,
		TYPE_WITH_ARGS,
		TYPE_MAX
	};

	/**
	 * Creates error descriptions.
	 */
	ErrorCodeDescriptions(StringView without_args,
						  StringView with_args) :
		m_descriptions { without_args, with_args}
	{}

	/**
	 * Creates error descriptions.
	 */
	ErrorCodeDescriptions(StringView without_args) :
		ErrorCodeDescriptions(without_args, without_args)
	{}

	/**
	 * Gets error description.
	 */
	StringView get_description(DescriptionType description_type) const
	{
		return m_descriptions[description_type];
	}

private:
	StringView m_descriptions[TYPE_MAX];
};


/**
 * ErrorCodeCategory is a virtual base class of error category.
 * @details Why not use std::system_category() directly. Because it use std::string as return
 *          type of get a description of error and that will waste more time.
 */
class ErrorCodeCategory
{
public:
	ErrorCodeCategory() = default;
	virtual ~ErrorCodeCategory() = default;

	/**
	 * Returns name of category.
	 */
	virtual StringView name() const = 0;

	/**
	 * Returns error descriptions of error code.
	 */
	virtual const ErrorCodeDescriptions& descriptions(int code) const = 0;
};

/**
 * LightsErrorCodeCategory is a implementation of error category that use in this lights libraray.
 */
class LightsErrorCodeCategory: public ErrorCodeCategory
{
public:
	/**
	 * Returns name of category.
	 */
	StringView name() const override;

	/**
	 * Returns error descriptions of error code.
	 */
	const ErrorCodeDescriptions& descriptions(int code) const override;

	static LightsErrorCodeCategory& instance()
	{
		static LightsErrorCodeCategory category;
		return category;
	}
};


/**
 * Exception is the base class of all exception that use in this library.
 */
class Exception: public std::exception
{
public:
	/**
	 * Creates exception.
	 */
	Exception(const SourceLocation& occur_location, int code, const ErrorCodeCategory& code_category = LightsErrorCodeCategory::instance()):
		m_occur_location(occur_location), m_code(code), m_code_category(code_category)
	{
	}

	/**
	 * Returns the occur source location of this exception.
	 */
	const SourceLocation& occur_location() const
	{
		return m_occur_location;
	}

	/**
	 * Returns the error message with null-terminated string.
	 */
	const char* what() const noexcept override;

	/**
	 * Returns error code.
	 */
	int code() const
	{
		return m_code;
	}

	/**
	 * Returns error category.
	 */
	const ErrorCodeCategory& code_category() const
	{
		return m_code_category;
	}

	/**
	 * Gets description of error code.
	 */
	StringView get_description(ErrorCodeDescriptions::DescriptionType description_type) const
	{
		return code_category().descriptions(code()).get_description(description_type);
	}

	/**
	 * Dumps the error message to sink adapter @c out.
	 * Derived class can implement it via format with arguments.
	 * @note The error message is no include occur location.
	 */
	virtual void dump_message(SinkAdapter& out,
							  ErrorCodeDescriptions::DescriptionType description_type =
							  		ErrorCodeDescriptions::TYPE_WITH_ARGS) const;

private:
	SourceLocation m_occur_location;
	int m_code;
	const ErrorCodeCategory& m_code_category;
};


/**
 * AssertionError is throw by assertion failure.
 */
class AssertionError: public Exception
{
public:
	AssertionError(const SourceLocation& occur_location, StringView description):
		Exception(occur_location, error_code::ASSERTION_ERROR), m_description(description)
	{
	}

	void dump_message(SinkAdapter& out, ErrorCodeDescriptions::DescriptionType description_type) const override;

private:
	StringView m_description;
};


/**
 * InvalidArgument is throw by arguments is invalid.
 */
class InvalidArgument: public Exception
{
public:
	InvalidArgument(const SourceLocation& occur_location, StringView description):
		Exception(occur_location, error_code::INVALID_ARGUMENT), m_description(description.to_std_string())
	{
	}

	void dump_message(SinkAdapter& out, ErrorCodeDescriptions::DescriptionType description_type) const override;

private:
	std::string m_description;
};


/**
 * OpenFileError is throw by open file failure.
 */
class OpenFileError: public Exception
{
public:
	OpenFileError(const SourceLocation& occur_location, StringView filename):
		Exception(occur_location, error_code::OPEN_FILE_FAILURE), m_filename(filename.to_std_string())
	{
	}

	void dump_message(SinkAdapter& out, ErrorCodeDescriptions::DescriptionType description_type) const override;

private:
	std::string m_filename;
};


/**
 * Throws a exception and record the current source location.
 * @arg ... Expand to fmt and arg ...
 * @note Expand macro method is not standard.
 */
#define LIGHTS_THROW_EXCEPTION(ExceptionType, ...) \
        throw ExceptionType(LIGHTS_CURRENT_SOURCE_LOCATION, ##__VA_ARGS__);


#if LIGHTS_OPEN_ASSERTION == 1
#	define LIGHTS_ASSERT(expr) assert(expr)
#elif LIGHTS_OPEN_ASSERTION == 2
#	define LIGHTS_ASSERT(expr) \
	do { \
		if (!(expr)) \
			LIGHTS_THROW_EXCEPTION(lights::AssertionError, #expr); \
	} while (false);
#else
#	define LIGHTS_ASSERT(expr)
#endif

/**
 * Dumps all message of exception to sink adapter, include error message and occur source
 * location.
 */
void dump(const Exception& ex, SinkAdapter& out);

inline SinkAdapter& operator<< (SinkAdapter& out, const Exception& ex)
{
	dump(ex, out);
	return out;
}


namespace details {

template <typename Sink>
class FormatSelfSinkAdapter: public SinkAdapter
{
public:
	FormatSelfSinkAdapter(FormatSinkAdapter<Sink> out):
		m_out(out)
	{}

	std::size_t write(SequenceView sequence) override
	{
		m_out.append(to_string_view(sequence));
		return sequence.length();
	};
private:
	FormatSinkAdapter<Sink> m_out;
};

} // namespace details


/**
 * Converts exception to string and put to format sink.
 */
template <typename Sink>
inline void to_string(FormatSinkAdapter<Sink> out, const Exception& ex)
{
	details::FormatSelfSinkAdapter<Sink> adapter(out);
	adapter << ex;
}


class BinaryStoreWriter;

/**
 * Converts exception to string and put to format sink.
 */
void to_string(FormatSinkAdapter<BinaryStoreWriter> out, const Exception& ex);

} // namespace lights

/**
 * ostream.h
 * @author wherewindblow
 * @date   Aug 17, 2017
 *
 * Include this file to support operation with std::ostream.
 */

#pragma once

#include <streambuf>
#include <ostream>

#include "sequence.h"
#include "format.h"


namespace lights {
namespace details {

/**
 * StringBuffer that reference on a string.
 * Instead of std::stringbuf to optimize performance.
 */
template <typename Backend>
class StringBuffer: public std::streambuf
{
public:
	/**
	 * Creates string buffer.
	 */
	explicit StringBuffer(FormatSink<Backend>& sink);

	/**
	 * Inserts a character.
	 */
	virtual int_type overflow(int_type ch) override;

	/**
	 * Inserts multiple character.
	 */
	virtual std::streamsize	xsputn(const char* s, std::streamsize n) override;

private:
	FormatSink<Backend>& m_sink;
};


template <typename Backend>
StringBuffer<Backend>::StringBuffer(FormatSink <Backend>& sink) :
	m_sink(sink)
{}


template <typename Backend>
int StringBuffer<Backend>::overflow(int ch)
{
	m_sink.append(static_cast<char>(ch));
	return ch;
}


template <typename Backend>
std::streamsize StringBuffer<Backend>::xsputn(const char* s, std::streamsize n)
{
	m_sink.append({s, static_cast<std::size_t>(n)});
	return n;
}

} // namespace details


/**
 * To convert @c value to string in the end of @ sink
 * that use insertion operator with std::ostream and T.
 * Aim to support format with
 *   `std::ostream& operator<< (std::ostream& out, const T& value)`
 * @param sink   A FormatSink.
 * @param value  User-defined type value.
 */
template <typename Backend, typename T>
inline void to_string(FormatSink<Backend> sink, const T& value)
{
	details::StringBuffer<Backend> buf(sink);
	std::ostream ostream(&buf);
	ostream << value;
}

/**
 * Inserts string to ostream.
 */
inline std::ostream& operator<< (std::ostream& out, StringView str)
{
	out.write(str.data(), str.length());
	return out;
}

/**
 * Inserts sequence to ostream.
 */
inline std::ostream& operator<< (std::ostream& out, SequenceView sequence)
{
	out << to_string_view(sequence);
	return out;
}

} // namespace lights

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
template <typename Sink>
class StringBuffer: public std::streambuf
{
public:
	explicit StringBuffer(FormatSinkAdapter<Sink>& out) :
		m_out(out) {}

	virtual int_type overflow(int_type ch) override
	{
		m_out.append(static_cast<char>(ch));
		return ch;
	}

	virtual std::streamsize	xsputn(const char* s, std::streamsize n) override
	{
		m_out.append({s, static_cast<std::size_t>(n)});
		return n;
	}

private:
	FormatSinkAdapter<Sink>& m_out;
};

} // namespace details


/**
 * To convert @c value to string in the end of @ sink
 * that use insertion operator with std::ostream and T.
 * Aim to support format with
 *   `std::ostream& operator<< (std::ostream& out, const T& value)`
 * @param out    A FormatSinkAdapter.
 * @param value  User-defined type value.
 */
template <typename Sink, typename T>
inline void to_string(FormatSinkAdapter<Sink> out, const T& value)
{
	details::StringBuffer<Sink> buf(out);
	std::ostream ostream(&buf);
	ostream << value;
}


inline std::ostream& operator<< (std::ostream& out, StringView str)
{
	out.write(str.data(), str.length());
	return out;
}

inline std::ostream& operator<< (std::ostream& out, SequenceView sequence)
{
	out << to_string_view(sequence);
	return out;
}

} // namespace lights

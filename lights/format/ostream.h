/**
 * ostream.h
 * @author wherewindblow
 * @date   Aug 17, 2017
 */

#pragma once

#include "../format.h"

#include <streambuf>
#include <ostream>


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
	explicit StringBuffer(Sink& sink) :
		m_string(sink) {}

	virtual int_type overflow(int_type ch) override
	{
		m_string.append(static_cast<char>(ch));
		return ch;
	}

	virtual std::streamsize	xsputn(const char* s, std::streamsize n) override
	{
		m_string.append({s, static_cast<std::size_t>(n)});
		return n;
	}

private:
	FormatSinkAdapter<Sink>& m_string;
};

} // namespace details


/**
 * To convert @c value to string in the end of @ sink
 * that use insertion operator with std::ostream and T.
 * Aim to support format with
 *   `std::ostream& operator<< (std::ostream& out, const T& value)`
 * @param out    Abstract string.
 * @param value  User type.
 */
template <typename Sink, typename T>
inline void to_string(FormatSinkAdapter<Sink> out, const T& value)
{
	details::StringBuffer<Sink> buf(out);
	std::ostream ostream(&buf);
	ostream << value;
}

} // namespace lights

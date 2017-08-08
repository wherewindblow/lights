/**
 * block_description.h
 * @author wherewindblow
 * @date   Aug 06, 2017
 */

#pragma once

#include <cstring>
#include <string>


namespace lights {

class String
{
public:
	using CharType = char;

	String(CharType* str, std::size_t len) :
		m_data(str), m_length(len) {}

	String(CharType* str) :
		m_data(str), m_length(std::strlen(str)) {}

	String(std::string& str):
		m_data(&str.front()), m_length(str.length()) {}

	CharType* data()
	{
		return m_data;
	}

	const CharType* data() const
	{
		return m_data;
	}

	std::size_t length() const
	{
		return m_length;
	}

	std::string to_string() const
	{
		return std::string(m_data, m_length);
	}

	void move_forward(std::size_t len)
	{
		m_data += len;
		m_length -= len;
	}

private:
	CharType* m_data;
	std::size_t m_length;
};


/**
 * View of string, can reduce data copy.
 * @note Cannot store this at some place, because you cannot know where the
 *       resource that internal point to will not available. The best way to
 *       use it is use as parameter.
 */
class StringView
{
public:
	using CharType = const char;

	StringView(CharType* str, std::size_t len) :
		m_data(str), m_length(len) {}

	StringView(CharType* str) :
		m_data(str), m_length(std::strlen(str)) {}

	StringView(String str):
		m_data(str.data()), m_length(str.length()) {}

	StringView(const std::string& str):
		m_data(str.data()), m_length(str.length()) {}

	const CharType* data() const
	{
		return m_data;
	}

	std::size_t length() const
	{
		return m_length;
	}

	std::string to_string() const
	{
		return std::string(m_data, m_length);
	}

	void move_forward(std::size_t len)
	{
		m_data += len;
		m_length -= len;
	}

private:
	CharType* m_data;
	std::size_t m_length;
};


class Buffer
{
public:
	Buffer(void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	Buffer(String str) :
		m_data(str.data()), m_length(str.length()) {}

	void* data()
	{
		return m_data;
	}

	const void* data() const
	{
		return m_data;
	}

	std::size_t length() const
	{
		return m_length;
	}

	void move_forward(std::size_t len)
	{
		m_data = static_cast<std::uint8_t*>(m_data) + len;
		m_length -= len;
	}

private:
	void* m_data;
	std::size_t m_length;
};


/**
 * View of buffer, can reduce data copy.
 * @note Cannot store this at some place, because you cannot know where the
 *       resource that internal point to will not available. The best way to
 *       use it is use as parameter.
 */
class BufferView
{
public:
	BufferView(const void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	BufferView(Buffer buffer):
		m_data(buffer.data()), m_length(buffer.length()) {}

	BufferView(String str) :
		m_data(str.data()), m_length(str.length()) {}

	BufferView(StringView str) :
		m_data(str.data()), m_length(str.length()) {}

	const void* data() const
	{
		return m_data;
	}

	std::size_t length() const
	{
		return m_length;
	}

	void move_forward(std::size_t len)
	{
		m_data = static_cast<const std::uint8_t*>(m_data) + len;
		m_length -= len;
	}

private:
	const void* m_data;
	std::size_t m_length;
};



inline String to_string(Buffer buffer)
{
	return String(static_cast<String::CharType*>(buffer.data()), buffer.length());
}

inline StringView to_string_view(BufferView buffer)
{
	return StringView(static_cast<StringView::CharType*>(buffer.data()), buffer.length());
}


template <typename Ostream>
inline Ostream& operator<< (Ostream& out, StringView view)
{
	out.write(view.data(), view.length());
	return out;
}

template <typename Ostream>
inline Ostream& operator<< (Ostream& out, BufferView view)
{
	out << to_string_view(view);
	return out;
}

} // namespace lights

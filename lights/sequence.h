/**
 * sequence.h
 * @author wherewindblow
 * @date   Aug 06, 2017
 */

#pragma once

#include <cstdint>
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

	CharType& at(std::size_t index) const
	{
		return m_data[index];
	}

	CharType& operator[](std::size_t index)
	{
		return at(index);
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

	const CharType& at(std::size_t index) const
	{
		return m_data[index];
	}

	const CharType& operator[](std::size_t index) const
	{
		return at(index);
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


class Sequence
{
public:
	Sequence(void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	Sequence(String str) :
		m_data(str.data()), m_length(str.length()) {}

	void* data()
	{
		return m_data;
	}

	const void* data() const
	{
		return m_data;
	}

	template <typename T>
	T& at(std::size_t index)
	{
		return reinterpret_cast<T*>(m_data)[index];
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
 * View of sequence, can reduce data copy.
 * @note Cannot store this at some place, because you cannot know where the
 *       resource that internal point to will not available. The best way to
 *       use it is use as parameter.
 */
class SequenceView
{
public:
	SequenceView(const void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	SequenceView(Sequence buffer):
		m_data(buffer.data()), m_length(buffer.length()) {}

	SequenceView(String str) :
		m_data(str.data()), m_length(str.length()) {}

	SequenceView(StringView str) :
		m_data(str.data()), m_length(str.length()) {}

	const void* data() const
	{
		return m_data;
	}

	template <typename T>
	const T& at(std::size_t index) const
	{
		return reinterpret_cast<const T*>(m_data)[index];
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



inline String to_string(Sequence sequence)
{
	return String(static_cast<String::CharType*>(sequence.data()), sequence.length());
}

inline StringView to_string_view(SequenceView sequence)
{
	return StringView(static_cast<StringView::CharType*>(sequence.data()), sequence.length());
}

} // namespace lights

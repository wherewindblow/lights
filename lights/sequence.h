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

/**
 * Reference to a string, only contain a pointer and length.
 * It's can convert from @c std::string and literal string.
 */
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

	std::string to_std_string() const
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
 * Reference to a const string, only contain a pointer and length.
 * It's can convert from @c String, @c std::string and literal string.
 * @note Cannot store this at some place, because you cannot know where the
 *       resource that internal point to will not valid. The best way to
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

	std::string to_std_string() const
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
 * Reference to a void buffer, only contain a pointer and length.
 * It's can convert from @c String.
 */
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
 * Reference to a void buffer, only contain a pointer and length.
 * It's can convert from @c Sequence, @c String and @c StringView.
 * @note Cannot store this at some place, because you cannot know where the
 *       resource that internal point to will not valid. The best way to
 *       use it is use as parameter.
 */
class SequenceView
{
public:
	SequenceView(const void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	SequenceView(Sequence sequence):
		m_data(sequence.data()), m_length(sequence.length()) {}

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


#define LIGHTS_IS_INVALID_SEQUENCE(Type) \
inline bool is_valid(Type sequence) \
{ \
	return sequence.data() != nullptr; \
}

LIGHTS_IS_INVALID_SEQUENCE(String);
LIGHTS_IS_INVALID_SEQUENCE(StringView);
LIGHTS_IS_INVALID_SEQUENCE(Sequence);
LIGHTS_IS_INVALID_SEQUENCE(SequenceView);


inline String invalid_string()
{
	return String(nullptr, 0);
}

inline StringView invalid_string_view()
{
	return StringView(nullptr, 0);
}

inline Sequence invalid_sequence()
{
	return Sequence(nullptr, 0);
}

inline SequenceView invalid_sequence_view()
{
	return SequenceView(nullptr, 0);
}


template <typename T, std::size_t N>
inline Sequence make_sequence(T (&target)[N])
{
	return Sequence(target, sizeof(T) * N);
};

template <typename T, std::size_t N>
inline SequenceView make_sequence_view(const T (&target)[N])
{
	return SequenceView(target, sizeof(T) * N);
};

template <std::size_t N>
inline String make_string(String::CharType (&target)[N])
{
	return String(target, sizeof(String::CharType) * N);
};

template <std::size_t N>
inline StringView make_string_view(StringView::CharType (&target)[N])
{
	return StringView(target, sizeof(StringView::CharType) * N);
};

} // namespace lights

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
 * Reference to string, only contain a pointer and length.
 * It's can convert from @c std::string and literal string.
 */
class String
{
public:
	using CharType = char;

	/**
	 * Creates string.
	 */
	String(CharType* str, std::size_t len) :
		m_data(str), m_length(len) {}

	/**
	 * Creates string.
	 */
	String(CharType* str) :
		m_data(str), m_length(std::strlen(str)) {}

	/**
	 * Creates string.
	 */
	String(std::string& str):
		m_data(&str.front()), m_length(str.length()) {}

	/**
	 * Returns underlying string.
	 */
	CharType* data()
	{
		return m_data;
	}

	/**
	 * Returns underlying string.
	 */
	const CharType* data() const
	{
		return m_data;
	}

	/**
	 * Returns character reference.
	 */
	CharType& at(std::size_t index) const
	{
		return m_data[index];
	}

	/**
	 * Returns character reference.
	 */
	CharType& operator[](std::size_t index)
	{
		return at(index);
	}

	/**
	 * Returns length of string.
	 */
	std::size_t length() const
	{
		return m_length;
	}

	/**
	 * Converts to std::string.
	 */
	std::string to_std_string() const
	{
		return std::string(m_data, m_length);
	}

	/**
	 * Moves string data address forward.
	 */
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

	/**
	 * Creates string view.
	 */
	StringView(CharType* str, std::size_t len) :
		m_data(str), m_length(len) {}

	/**
	 * Creates string view.
	 */
	StringView(CharType* str) :
		m_data(str), m_length(std::strlen(str)) {}

	/**
	 * Creates string view.
	 */
	StringView(String str):
		m_data(str.data()), m_length(str.length()) {}

	/**
	 * Creates string view.
	 */
	StringView(const std::string& str):
		m_data(str.data()), m_length(str.length()) {}

	/**
	 * Returns underlying string.
	 */
	const CharType* data() const
	{
		return m_data;
	}

	/**
	 * Returns character reference.
	 */
	const CharType& at(std::size_t index) const
	{
		return m_data[index];
	}

	/**
	 * Returns character reference.
	 */
	const CharType& operator[](std::size_t index) const
	{
		return at(index);
	}

	/**
	 * Returns length of string.
	 */
	std::size_t length() const
	{
		return m_length;
	}

	/**
	 * Converts to std::string.
	 */
	std::string to_std_string() const
	{
		return std::string(m_data, m_length);
	}

	/**
	 * Moves string data address forward.
	 */
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
	/**
	 * Creates sequence.
	 */
	Sequence(void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	/**
	 * Creates sequence.
	 */
	Sequence(String str) :
		m_data(str.data()), m_length(str.length()) {}

	/**
	 * Returns underlying data.
	 */
	void* data()
	{
		return m_data;
	}

	/**
	 * Returns underlying data.
	 */
	const void* data() const
	{
		return m_data;
	}

	/**
	 * Returns expect type reference.
	 */
	template <typename T>
	T& at(std::size_t index)
	{
		return reinterpret_cast<T*>(m_data)[index];
	}

	/**
	 * Returns length of sequence.
	 */
	std::size_t length() const
	{
		return m_length;
	}

	/**
	 * Moves string data address forward.
	 */
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
	/**
	 * Creates sequence view.
	 */
	SequenceView(const void* data, std::size_t len) :
		m_data(data), m_length(len) {}

	/**
	 * Creates sequence view.
	 */
	SequenceView(Sequence sequence):
		m_data(sequence.data()), m_length(sequence.length()) {}

	/**
	 * Creates sequence view.
	 */
	SequenceView(String str) :
		m_data(str.data()), m_length(str.length()) {}

	/**
	 * Creates sequence view.
	 */
	SequenceView(StringView str) :
		m_data(str.data()), m_length(str.length()) {}

	/**
	 * Returns underlying data.
	 */
	const void* data() const
	{
		return m_data;
	}

	/**
	 * Returns expect type reference.
	 */
	template <typename T>
	const T& at(std::size_t index) const
	{
		return reinterpret_cast<const T*>(m_data)[index];
	}

	/**
	 * Returns length of sequence.
	 */
	std::size_t length() const
	{
		return m_length;
	}

	/**
	 * Moves string data address forward.
	 */
	void move_forward(std::size_t len)
	{
		m_data = static_cast<const std::uint8_t*>(m_data) + len;
		m_length -= len;
	}

private:
	const void* m_data;
	std::size_t m_length;
};


/**
 * Converts sequence to string.
 */
inline String to_string(Sequence sequence)
{
	return String(static_cast<String::CharType*>(sequence.data()), sequence.length());
}

/**
 * Converts sequence view to string view.
 */
inline StringView to_string_view(SequenceView sequence)
{
	return StringView(static_cast<StringView::CharType*>(sequence.data()), sequence.length());
}


/**
 * Checks sequence is valid.
 */
#define LIGHTSIMPL_IS_INVALID_SEQUENCE(Type) \
inline bool is_valid(Type sequence) \
{ \
	return sequence.data() != nullptr; \
}

LIGHTSIMPL_IS_INVALID_SEQUENCE(String);
LIGHTSIMPL_IS_INVALID_SEQUENCE(StringView);
LIGHTSIMPL_IS_INVALID_SEQUENCE(Sequence);
LIGHTSIMPL_IS_INVALID_SEQUENCE(SequenceView);

#undef LIGHTSIMPL_IS_INVALID_SEQUENCE

/**
 * Returns invalid string.
 */
inline String invalid_string()
{
	return String(nullptr, 0);
}

/**
 * Returns invalid string view.
 */
inline StringView invalid_string_view()
{
	return StringView(nullptr, 0);
}

/**
 * Returns invalid sequence.
 */
inline Sequence invalid_sequence()
{
	return Sequence(nullptr, 0);
}

/**
 * Returns invalid sequence view.
 */
inline SequenceView invalid_sequence_view()
{
	return SequenceView(nullptr, 0);
}


/**
 * Creates sequence with any type array.
 */
template <typename T, std::size_t N>
inline Sequence make_sequence(T (&target)[N])
{
	return Sequence(target, sizeof(T) * N);
};

/**
 * Creates sequence view with any type array.
 */
template <typename T, std::size_t N>
inline SequenceView make_sequence_view(const T (&target)[N])
{
	return SequenceView(target, sizeof(T) * N);
};

/**
 * Creates string with char array.
 */
template <std::size_t N>
inline String make_string(String::CharType (&target)[N])
{
	return String(target, sizeof(String::CharType) * N);
};

/**
 * Creates string view with char array.
 */
template <std::size_t N>
inline StringView make_string_view(StringView::CharType (&target)[N])
{
	return StringView(target, sizeof(StringView::CharType) * N);
};


/**
 * Compares string view is equal.
 */
inline bool operator==(StringView lhs, StringView rhs)
{
	return lhs.length() == rhs.length() && std::strncmp(lhs.data(), rhs.data(), lhs.length()) == 0;
}

/**
 * Compares string view is not equal.
 */
inline bool operator!=(StringView lhs, StringView rhs)
{
	return !(lhs == rhs);
}

} // namespace lights

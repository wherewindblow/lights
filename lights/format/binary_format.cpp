/**
 * binary_format.cpp
 * @author wherewindblow
 * @date   Sep 12, 2017
 */

#include "binary_format.h"


namespace lights {

/**
 * Gets type code of boolean.
 */
inline BinaryTypeCode get_type_code(bool)
{
	return BinaryTypeCode::BOOL;
}

/**
 * Gets type code of char.
 */
inline BinaryTypeCode get_type_code(char)
{
	return BinaryTypeCode::CHAR;
}

/**
 * Gets type code of string.
 */
inline BinaryTypeCode get_type_code(const char*)
{
	return BinaryTypeCode::STRING;
}

/**
 * Get @c BinaryTypeCode by integer type @c T.
 */
template <typename T>
inline BinaryTypeCode get_type_code(T)
{
	int offset;
	switch (std::numeric_limits<std::make_unsigned_t<T>>::digits)
	{
		case 8:
			offset = 0;
			break;
		case 16:
			offset = 1;
			break;
		case 32:
			offset = 2;
			break;
		case 64:
			offset = 3;
			break;
		default:
			offset = 0;
			break;
	}

	offset *= 2;
	offset += !std::numeric_limits<T>::is_signed ? 1 : 0;
	return static_cast<BinaryTypeCode>(static_cast<int>(BinaryTypeCode::INT8_T) + offset);
}

/**
 * Get type with of type by BinaryTypeCode.
 */
inline std::uint8_t get_type_width(BinaryTypeCode code)
{
	static std::uint8_t widths[] = {
		0, // Invalid.
		1, 1, // Bool and char
		1,    // String.
		1, 1, // 8 bits
		2, 2, // 16 bits
		4, 4, // 32 bits
		8, 8, // 64 bits
		2,    // User-define composed type
		4,    // String reference only store a string table index.
	};

	std::uint8_t index = static_cast<std::uint8_t>(code);
	if (index < 0 || index >= static_cast<std::uint8_t>(BinaryTypeCode::MAX))
	{
		return widths[0];
	}
	else
	{
		return widths[index];
	}
}


BinaryStoreWriter::BinaryStoreWriter(Sequence write_target, StringTable* str_table_ptr) :
	m_use_default_buffer(!is_valid(write_target)),
	m_buffer(is_valid(write_target) ?
			 static_cast<std::uint8_t*>(write_target.data()) :
			 new std::uint8_t[WRITER_BUFFER_SIZE_DEFAULT]),
	m_length(0),
	m_capacity(is_valid(write_target) ? write_target.length() : WRITER_BUFFER_SIZE_DEFAULT),
	m_state(FormatComposedTypeState::NO_INIT),
	m_composed_member_num(0),
	m_str_table_ptr(str_table_ptr)
{}


BinaryStoreWriter::BinaryStoreWriter(const BinaryStoreWriter& rhs)
{
	*this = rhs;
}


BinaryStoreWriter::~BinaryStoreWriter()
{
	if (m_use_default_buffer)
	{
		delete[] m_buffer;
	}
}


BinaryStoreWriter& BinaryStoreWriter::operator=(const BinaryStoreWriter& rhs)
{
	if (&rhs != this)
	{
		if (m_use_default_buffer)
		{
			delete[] m_buffer;
		}

		m_use_default_buffer = rhs.m_use_default_buffer;
		m_buffer = rhs.m_use_default_buffer ? new std::uint8_t[WRITER_BUFFER_SIZE_DEFAULT] : rhs.m_buffer;
		m_length = rhs.m_length;
		m_capacity = rhs.m_capacity;
		m_state = rhs.m_state;
		m_composed_member_num = rhs.m_composed_member_num;
		m_str_table_ptr = rhs.m_str_table_ptr;
	}
	return *this;
}


void BinaryStoreWriter::append(char ch)
{
	if (can_append(sizeof(BinaryTypeCode) + sizeof(ch)))
	{
		if (m_state == FormatComposedTypeState::STARTED)
		{
			++m_composed_member_num;
		}
		m_buffer[m_length++] = static_cast<std::uint8_t>(BinaryTypeCode::CHAR);
		m_buffer[m_length++] = static_cast<std::uint8_t>(ch);
	}
}


void BinaryStoreWriter::append(StringView str, bool store_in_table)
{
	if (str.length() == 0)
	{
		return;
	}
	else if (str.length() == 1)
	{
		append(str[0]);
	}
	else
	{
		// Store string in string table.
		if (store_in_table && m_str_table_ptr)
		{
			BinaryTypeCode type_code = BinaryTypeCode::STRING_REF;
			if (can_append(sizeof(BinaryTypeCode) + get_type_width(type_code)))
			{
				if (m_state == FormatComposedTypeState::STARTED)
				{
					++m_composed_member_num;
				}
				m_buffer[m_length++] = static_cast<std::uint8_t>(type_code);
				std::uint32_t* p = reinterpret_cast<std::uint32_t *>(&m_buffer[m_length]);
				auto index = static_cast<std::uint32_t>(m_str_table_ptr->get_index(str));
				*p = index;
				m_length += get_type_width(type_code);
			}
		}
		// Store string in buffer as other value.
		else if (can_append(str.length() + sizeof(BinaryTypeCode) + sizeof(std::uint8_t)))
		{
			if (m_state == FormatComposedTypeState::STARTED)
			{
				++m_composed_member_num;
			}
			m_buffer[m_length++] = static_cast<std::uint8_t>(BinaryTypeCode::STRING);
			m_buffer[m_length++] = static_cast<std::uint8_t>(str.length());
			std::memcpy(m_buffer + m_length, str.data(), str.length());
			m_length += str.length();
		}
	}
}


#define LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(Type)        \
	{                                                                   \
		BinaryTypeCode type_code = get_type_code(n);                    \
		if (can_append(sizeof(BinaryTypeCode) + get_type_width(type_code))) \
		{                                                               \
			if (m_state == FormatComposedTypeState::STARTED)            \
			{                                                           \
				++m_composed_member_num;                                \
			}                                                           \
			m_buffer[m_length++] = static_cast<std::uint8_t>(type_code);\
			Type* p = reinterpret_cast<Type *>(&m_buffer[m_length]);    \
			*p = n;                                                     \
			m_length += get_type_width(type_code);                      \
		}                                                               \
		return *this;                                                   \
	}

BinaryStoreWriter& BinaryStoreWriter::operator<< (std::int8_t n) \
{
	LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(std::int8_t);
}

BinaryStoreWriter& BinaryStoreWriter::operator<< (std::uint8_t n) \
{
	LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(std::uint8_t);
}


/**
 * Inserts a integer and convert to small integer type when can convert.
 */
#define LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(Type, FitSmallType) \
	BinaryStoreWriter& BinaryStoreWriter::operator<< (Type n)          \
	{                                                                  \
		if (n > std::numeric_limits<FitSmallType>::max() || n < std::numeric_limits<FitSmallType>::min()) \
		{                                                              \
			LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(Type);  \
		}                                                              \
		else                                                           \
		{                                                              \
			return *this << static_cast<FitSmallType>(n);              \
		}                                                              \
	}

LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(std::int16_t, std::int8_t);
LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(std::int32_t, std::int16_t);
LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(std::int64_t, std::int32_t);

#undef LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER


#define LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(Type, FitSmallType) \
	BinaryStoreWriter& BinaryStoreWriter::operator<< (Type n)         \
	{                                                                 \
		if (n > std::numeric_limits<FitSmallType>::max())             \
		{                                                             \
			LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(Type); \
		}                                                             \
		else                                                          \
		{                                                             \
			return *this << static_cast<FitSmallType>(n);             \
		}                                                             \
	}

LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(std::uint16_t, std::uint8_t);
LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(std::uint32_t, std::uint16_t);
LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(std::uint64_t, std::uint32_t);

#undef LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER

#undef LIGHTSIMPL_BINARY_STORE_WRITER_APPEND_INTEGER_BODY


void BinaryRestoreWriter::write_binary(StringView fmt, const std::uint8_t* binary_store_args, std::size_t args_length)
{
	if (args_length == 0)
	{
		append(fmt);
		return;
	}

	std::size_t i = 0;
	for (; i < fmt.length(); ++i)
	{
		if (fmt[i] == '{' &&
			i + 1 < fmt.length() &&
			fmt[i+1] == '}')
		{
			break;
		}
	}

	m_writer.append({fmt.data(), i});
	if (i < fmt.length())
	{
		auto width = write_argument(binary_store_args);
		fmt.move_forward(i + 2); // Skip "{}".
		write_binary(fmt, binary_store_args + width, args_length - width);
	}
}


std::uint8_t BinaryRestoreWriter::write_argument(const std::uint8_t* binary_store_args)
{
	auto width = get_type_width(static_cast<BinaryTypeCode>(*binary_store_args));
	auto value_begin = binary_store_args + sizeof(BinaryTypeCode);
	switch (static_cast<BinaryTypeCode>(*binary_store_args))
	{
		case BinaryTypeCode::INVALID:
			break;
		case BinaryTypeCode::BOOL:
		{
			bool b = static_cast<bool>(*value_begin);
			m_writer << b;
			break;
		}
		case BinaryTypeCode::CHAR:
		{
			char ch = static_cast<char>(*value_begin);
			m_writer << ch;
			break;
		}
		case BinaryTypeCode::STRING:
		{
			width += binary_store_args[1];
			m_writer.append({reinterpret_cast<const char*>(&binary_store_args[2]), binary_store_args[1]});
			break;
		}
		case BinaryTypeCode::INT8_T:
		{
			auto p = reinterpret_cast<const int8_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT8_T:
		{
			auto p = value_begin;
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT16_T:
		{
			auto p = reinterpret_cast<const int16_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT16_T:
		{
			auto p = reinterpret_cast<const uint16_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT32_T:
		{
			auto p = reinterpret_cast<const int32_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT32_T:
		{
			auto p = reinterpret_cast<const uint32_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT64_T:
		{
			auto p = reinterpret_cast<const int64_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT64_T:
		{
			auto p = reinterpret_cast<const uint64_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::COMPOSED_TYPE:
		{
			auto member_num = reinterpret_cast<const uint16_t*>(value_begin);
			for (std::size_t i = 0; i < *member_num; ++i)
			{
				width += write_argument(binary_store_args + sizeof(BinaryTypeCode) + width);
			}
			break;
		}
		case BinaryTypeCode::STRING_REF:
		{
			auto index = reinterpret_cast<const uint32_t*>(value_begin);
			if (m_str_table_ptr)
			{
				StringView str = m_str_table_ptr->get_str(*index);
				if (is_valid(str))
				{
					m_writer << str;
				}
				else
				{
					m_writer << "[[Invalid string index: " << *index << "]]";
				}
			}
			else
			{
				m_writer << "[[Use STRING_REF but string table is not set]]";
			}
			break;
		}
		case BinaryTypeCode::MAX:
			break;
	}
	return width + sizeof(BinaryTypeCode);
}

} // namespace lights

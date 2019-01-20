/**
 * binary_format.cpp
 * @author wherewindblow
 * @date   Sep 12, 2017
 */

#include "binary_format.h"


namespace lights {

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
		// TODO: How to store string when don't need to store in table?
	}
}


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

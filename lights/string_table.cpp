/**
 * string_table.cpp
 * @author wherewindblow
 * @date   Aug 30, 2017
 */

#include "string_table.h"

#include <vector>
#include <unordered_map>
#include <fstream>

#include "exception.h"
#include "env.h"


namespace lights {

namespace details {

struct StringTableImpl
{
	using StringViewPtr = std::shared_ptr<const StringView>;

	struct StringHash
	{
		size_t operator()(const StringViewPtr& str) const noexcept
		{
			return env_hash(str->data(), str->length());
		}
	};

	struct StringEqualTo
	{
		bool operator()(const StringViewPtr& lhs, const StringViewPtr& rhs) const noexcept
		{
			if (lhs->length() != rhs->length())
			{
				return false;
			}
			else
			{
				return std::memcmp(lhs->data(), rhs->data(), rhs->length()) == 0;
			}
		}
	};

	struct EmptyDeleter
	{
		void operator()(const StringView*) const noexcept {}
	};

	struct StringDeleter
	{
		void operator()(const StringView* str) const noexcept
		{
			delete[] str->data();
			delete str;
		}
	};

	std::fstream storage_file;
	std::size_t last_index = static_cast<std::size_t>(-1);
	std::vector<StringViewPtr> str_array; // To generate index
	std::unordered_map<StringViewPtr,
					   std::uint32_t,
					   StringHash,
					   StringEqualTo> str_hash; // To find faster.
};

} // namespace details

StringTable::StringTable(StringView filename):
	p_impl(std::make_unique<ImplementType>())
{
	impl()->storage_file.open(filename.data());
	if (impl()->storage_file.is_open())
	{
		std::string line;
		while (std::getline(impl()->storage_file, line))
		{
			add_str(StringView(line.c_str(), line.length()));
		}
		impl()->last_index = impl()->str_array.size() - 1;
	}
	else
	{
		impl()->storage_file.open(filename.data(), std::ios_base::out); // Create file.
		if (!impl()->storage_file.is_open())
		{
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}
}


StringTable::~StringTable()
{
	if (impl()->storage_file.is_open())
	{
		impl()->storage_file.seekp(0, std::ios_base::end);
		impl()->storage_file.clear();
		for (std::size_t i = impl()->last_index + 1; impl()->storage_file && i < impl()->str_array.size(); ++i)
		{
			impl()->storage_file.write(impl()->str_array[i]->data(), impl()->str_array[i]->length());
			impl()->storage_file << LIGHTS_LINE_ENDER;
		}
		impl()->storage_file.close();
	}
}


std::size_t StringTable::get_index(StringView str)
{
	ImplementType::StringViewPtr str_ptr(&str, ImplementType::EmptyDeleter());
	auto itr = impl()->str_hash.find(str_ptr);
	if (itr == impl()->str_hash.end())
	{
		return add_str(str);
	}
	else
	{
		return itr->second;
	}
}


std::size_t StringTable::add_str(StringView str)
{
	char* storage = new char[str.length()];
	copy_array(storage, str.data(), str.length());
	StringView* new_view = new StringView(storage, str.length());
	ImplementType::StringViewPtr str_ptr(new_view, ImplementType::StringDeleter());

	impl()->str_array.push_back(str_ptr);
	auto pair = std::make_pair(str_ptr, impl()->str_array.size() - 1);
	impl()->str_hash.insert(pair);
	return pair.second;
}


StringView StringTable::get_str(std::size_t index) const
{
	if (index < impl()->str_array.size())
	{
		return *impl()->str_array[index];
	}
	else
	{
		return invalid_string_view();
	}
}

} // namespace lights

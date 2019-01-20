/**
 * string_table.cpp
 * @author wherewindblow
 * @date   Aug 30, 2017
 */

#include "string_table.h"

#include <vector>
#include <unordered_map>
#include <fstream>

#include "config.h"
#include "env.h"
#include "exception.h"

#include <memory>
namespace lights {

namespace details {

struct StringTableImpl
{
	struct StringHash
	{
		size_t operator()(StringView str) const noexcept
		{
			return env::hash(str.data(), str.length());
		}
	};

	struct StringEqualTo
	{
		bool operator()(StringView lhs, StringView rhs) const noexcept
		{
			if (lhs.length() != rhs.length())
			{
				return false;
			}
			else
			{
				return std::memcmp(lhs.data(), rhs.data(), rhs.length()) == 0;
			}
		}
	};

	std::fstream storage_file;
	std::size_t last_index = static_cast<std::size_t>(-1);
	std::vector<StringView> str_array; // To generate index.
	std::unordered_map<StringView,
					   std::uint32_t, // index of str_array.
					   StringHash,
					   StringEqualTo> str_hash; // To find faster.
};

} // namespace details


StringTable::StringTable(StringView filename):
	p_impl(new ImplementType())
{
	p_impl->storage_file.open(filename.data());
	if (p_impl->storage_file.is_open())
	{
		std::string line;
		while (std::getline(p_impl->storage_file, line))
		{
			add_str(StringView(line.c_str(), line.length()));
		}
		p_impl->last_index = p_impl->str_array.size() - 1;
	}
	else
	{
		p_impl->storage_file.open(filename.data(), std::ios_base::out); // Create file.
		if (!p_impl->storage_file.is_open())
		{
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}
}


StringTable::~StringTable()
{
	if (p_impl->storage_file.is_open())
	{
		p_impl->storage_file.seekp(0, std::ios_base::end);
		p_impl->storage_file.clear();
		for (std::size_t i = p_impl->last_index + 1; p_impl->storage_file && i < p_impl->str_array.size(); ++i)
		{
			StringView str = p_impl->str_array[i];
			p_impl->storage_file.write(str.data(), str.length());
			p_impl->storage_file << env::end_line();
		}
		p_impl->storage_file.close();
	}

	for (auto& str : p_impl->str_array)
	{
		delete[] str.data();
	}

	delete p_impl;
}


std::size_t StringTable::get_index(StringView str)
{
	auto itr = p_impl->str_hash.find(str);
	if (itr == p_impl->str_hash.end())
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
	StringView new_str(storage, str.length());

	p_impl->str_array.push_back(new_str);
	auto pair = std::make_pair(new_str, p_impl->str_array.size() - 1);
	p_impl->str_hash.insert(pair);
	return pair.second;
}


StringView StringTable::get_str(std::size_t index) const
{
	if (index < p_impl->str_array.size())
	{
		return p_impl->str_array[index];
	}
	else
	{
		return invalid_string_view();
	}
}

} // namespace lights

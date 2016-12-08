/*
 * format.cpp
 * @author wherewindblow
 * @date   Dec 08, 2016
 */

#include "format.h"

#include <limits>


namespace lights {
namespace details {

void to_string(std::string& sink, short n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<short>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%hd", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, unsigned short n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<unsigned short>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%hu", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, int n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<int>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%d", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, unsigned int n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<unsigned int>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%u", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, long n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<long>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%ld", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, unsigned long n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<unsigned long>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%lu", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, long long n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<long long>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%lld", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, unsigned long long n)
{
	auto length = sink.length();
	sink.resize(sink.length() + std::numeric_limits<unsigned long long>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%llu", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, float n)
{
	auto length = sink.length();
	sink.resize(sink.length() +
		std::numeric_limits<float>::max_exponent10 + 1 +
		std::numeric_limits<float>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%f", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, double n)
{
	auto length = sink.length();
	// 100 is the max exponent10 of double that can convert in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	sink.resize(sink.length() +
		100 + 1 +
		std::numeric_limits<double>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%lf", n);
	sink.resize(length + writen);
}

void to_string(std::string& sink, long double n)
{
	auto length = sink.length();
	// 100 is the max exponent10 of long double that can convert in
	// sprintf on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2).
	sink.resize(sink.length() +
		100 + 1 +
		std::numeric_limits<long double>::digits10 + 1);
	auto ptr = &sink[length];
	int writen = std::sprintf(ptr, "%Lf", n);
	sink.resize(length + writen);
}

} // namespace details
} // namespace lights
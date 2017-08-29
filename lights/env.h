/**
 * env.h
 * @author wherewindblow
 * @date   Aug 27, 2017
 */

#pragma once

#include <cstddef>
#include <ctime>

#include <stdio.h>
#include <string.h>
#include <time.h>


namespace lights {

// In XSI-compliant (posix) version, 100 charater is enough to put all error
// string on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2) (Englist Version).
// In another languague version may have to change to largger to
// hold all message.
// In GNU-specific version 100 charater can hold all unkown error.
constexpr int ENV_MAX_ERROR_STR_LEN = 100;

/**
 * Return string version of error number.
 * @param error_no  Enum of error.
 * @param buf       Use to hold error string sometime.
 * @note Must use return value as result, not @c buf. Because @c buf
 *       is not use sometime.
 */
inline const char* env_strerror(int error_no, char* buf, std::size_t len)
{
	// Not use sys_nerr and sys_errlist directly, although the are easy to control.
	// Because sys_nerr may bigger that sys_errlist size and sys_errlist may have't
	// all string for errno. In the way will lead to segment fault.
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE  // XSI-compliant (posix) version.
	if (strerror_r(errer_no, m_buf, sizeof(m_buf)) == 0)
	{
			return m_buf;
	}
	else
	{
		return "Unkown error";
	}
#else // GNU-specific version, buf only use when it is unkown.
	return strerror_r(error_no, buf, sizeof(len));
#endif
}


using env_offset_t = std::ptrdiff_t;

inline env_offset_t env_ftell(std::FILE* file)
{
	// Return type off_t will fit into suitable type for 32 and 64 architechures.
	return ftello(file);
}

inline void env_fseek(std::FILE* file, env_offset_t off, int whence)
{
	// off type off_t will fit into suitable type for 32 and 64 architechures.
	fseeko(file, off, whence);
}

inline tm* env_localtime(const std::time_t* time_point, struct tm* result)
{
	return localtime_r(time_point, result);
}

inline std::size_t env_hash(const void* data, std::size_t len)
{
	return std::_Hash_impl::hash(data, len);
}

} // namespace lights

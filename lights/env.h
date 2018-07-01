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
#include <unistd.h>


namespace lights {
namespace env {

/**
 * Sets pod type do not memory alignment.
 */
#define LIGHTS_NOT_MEMEORY_ALIGNMENT __attribute__((packed))


/**
 * End line string.
 */
inline const char* end_line()
{
	return "\n";
}

// In XSI-compliant (posix) version, 100 character is enough to put all error
// string on g++ (GCC) 6.2.1 20160916 (Red Hat 6.2.1-2) (Englist Version).
// In another languague version may have to change to largger to
// hold all message.
// In GNU-specific version 100 character can hold all unkown error.
constexpr int MAX_ERROR_STR_LEN = 100;

/**
 * Return string version of error number.
 * @param error_no  Enum of error.
 * @param buf       Use to hold error string sometime.
 * @note Must use return value as result, not @c buf. Because @c buf
 *       is not use sometime.
 */
inline const char* strerror(int error_no, char* buf, std::size_t len)
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


using offset_t = std::ptrdiff_t;

/**
 * Tells file offset.
 */
inline offset_t ftell(std::FILE* file)
{
	// Return type off_t will fit into suitable type for 32 and 64 architechures.
	return ftello(file);
}

/**
 * Seeks file offset.
 */
inline void fseek(std::FILE* file, offset_t off, int whence)
{
	// Return type off_t will fit into suitable type for 32 and 64 architechures.
	fseeko(file, off, whence);
}

/**
 * Gets tm by timestamp.
 */
inline tm* localtime(const std::time_t* time_point, struct tm* result)
{
	return localtime_r(time_point, result);
}

/**
 * Computes hash of data.
 */
inline std::size_t hash(const void* data, std::size_t len)
{
	return std::_Hash_impl::hash(data, len);
}

/**
 * Checks file is exists.
 */
inline bool file_exists(const char* filename)
{
	return access(filename, F_OK) == 0;
}

} // namespace env
} // namespace lights

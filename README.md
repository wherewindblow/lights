# Lights Library
Light weight and light speed library set.
To ensure user can use feature by the way that user want. So don't to do more management.
If user want to manage logger and etc, must manage manually.

## Library List
- **format**  Type-safe format and similar to the one used by str.format in Python.
- **log**     Logging library with use format library to format message.

## Features
- **format**
	- Basic write API with format string syntax similar to the one used by str.format in Python.
	- Write API similar to the one used by iostreams.
	- Simple wrapper format API can easy use and return std::string.
	- Support variadic argument to format.
	- Support user-defined type to format.
	- High speed and close or better performance to std::printf.
	- Allow to use adapter to adapt user-defined type as sink of format.

- **log**
	- Hight performance.
	- Custom formatting with log message.
	- Conditional logging with log message.
	- Various log target
		- Size rotating log files.
		- Time rotating log files.
	- Text logger with more readable.
	- Binary logger with save io and record more information with less space.

## Install
- Copy the source folder to your build tree and use a C++14 compiler.

## Dependency
- Library have not dependency.
- Benchmark and example have the following dependency.
	- Google benchmark
	- boost
	- fmt
	- spdlog

## Platform
- Only support linux now.

## Example
- See the folder [example](https://github.com/wherewindblow/lights/tree/master/example).
/**
 * bm_exception.cpp
 * @author wherewindblow
 * @date   Jul 05, 2017
 */

#include "bm_exception.h"

#include <system_error>
#include <boost/exception/all.hpp>

#include <benchmark/benchmark.h>
#include <lights/format.h>
#include <lights/exception.h>
#include <lights/format/binary_format.h>


#define EXCEPTION_FORMAT_STR "Have error at {} and it's {}"
#define EXCEPTION_FORMAT_ARGS EXCEPTION_FORMAT_STR, __LINE__, padding_str


namespace lights {
namespace benchmark {

class ExceptionBinary: public std::exception
{
public:
	ExceptionBinary(StringView what, bool store_what = false):
		m_what(what),
		m_store_what(store_what),
		m_store_writer(Sequence(m_store_write_target, sizeof(m_store_write_target)))
	{
		if (m_store_what)
		{
			char* data = new char[m_what.length()];
			m_what = StringView(data, m_what.length());
		}
	}

	ExceptionBinary(const ExceptionBinary& rhs):
		m_what(rhs.m_what),
		m_store_what(rhs.m_store_what),
		m_store_writer(rhs.m_store_writer),
		m_have_format(rhs.m_have_format),
		m_restore_write_target(rhs.m_restore_write_target.get() ?
							   (new char[WRITER_BUFFER_SIZE_DEFAULT]) : nullptr)
	{
		if (rhs.m_restore_write_target.get())
		{
			copy_array(m_restore_write_target.get(),
					   rhs.m_restore_write_target.get(),
					   WRITER_BUFFER_SIZE_DEFAULT);
		}
	}

	~ExceptionBinary()
	{
		if (m_store_what)
		{
			delete[] m_what.data();
		}
	}

	template <typename ... Arg>
	ExceptionBinary& add_arg(const Arg& ... arg)
	{
		m_store_writer.write(m_what, arg ...);
		return *this;
	}

	const char* what() const noexcept override
	{
		if (!m_have_format)
		{
			m_restore_write_target.reset(new char[WRITER_BUFFER_SIZE_DEFAULT]);
			String target(m_restore_write_target.get(), WRITER_BUFFER_SIZE_DEFAULT);
			BinaryRestoreWriter restore_writer(target);
			restore_writer.write_binary(m_what, m_store_writer.data(), m_store_writer.length());
			m_have_format = true;
		}

		return m_restore_write_target.get();
	}

private:
	StringView m_what;
	bool m_store_what;
	char m_store_write_target[50]; // Can hold 10 int parameter.
	BinaryStoreWriter m_store_writer;
	mutable bool m_have_format = false;
	mutable std::unique_ptr<char[]> m_restore_write_target;

};


class ExceptionText: public std::exception
{
public:
	ExceptionText(StringView what, bool store_what = false):
		m_what(what), m_store_what(store_what), m_writer({m_write_target, sizeof(m_write_target)})
	{
		if (m_store_what)
		{
			char* data = new char[m_what.length()];
			m_what = StringView(data, m_what.length());
		}
	}

	ExceptionText(const ExceptionText& rhs):
		m_what(rhs.m_what),
		m_store_what(rhs.m_store_what),
		m_writer(rhs.m_writer)
	{
	}

	~ExceptionText()
	{
		if (m_store_what)
		{
			delete[] m_what.data();
		}
	}

	template <typename ... Arg>
	ExceptionText& add_arg(const Arg& ... arg)
	{
		m_writer.write(m_what, arg ...);
		return *this;
	}

	const char* what() const noexcept override
	{
		return m_writer.c_str();
	}

private:
	StringView m_what;
	bool m_store_what;
	char m_write_target[WRITER_BUFFER_SIZE_DEFAULT];
	TextWriter m_writer;
};

} // namespace benchmark
} // namespace lights


#define LIGHTS_BENCHMARK_EXCEPTION_FORMAT
#define LIGHTS_BENCHMARK_EXCEPTION_FORMAT_ARG


// ----------------------- Exception. ------------------------

void BM_exception_int(benchmark::State& state)
{
	int perform_what = state.range(0);
	while (state.KeepRunning())
	{
		try
		{
			int code(2);
			throw code;
		}
		catch (const int& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex);
			}
		}
	}
}


void BM_exception_std_error_code(benchmark::State& state)
{
	int perform_what = state.range(0);
	while (state.KeepRunning())
	{
		try
		{
			std::error_code code(static_cast<int>(std::errc::address_not_available), std::system_category());
			throw code;
		}
		catch (const std::error_code& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex.message());
			}
		}
	}
}


void BM_exception_lights_Exception(benchmark::State& state)
{
	lights::NullSink null;
	int perform_what = state.range(0);
	while (state.KeepRunning())
	{
		try
		{
			LIGHTS_THROW_EXCEPTION(lights::Exception, lights::error_code::OPEN_FILE_FAILURE);
		}
		catch (const lights::Exception& ex)
		{
			if (perform_what)
			{
				ex.dump_message(null);
			}
		}
	}
}


void BM_exception_str_std_exception(benchmark::State& state)
{
	int perform_what = state.range(0);
	while (state.KeepRunning())
	{
		try
		{
			LIGHTS_DEFAULT_TEXT_WRITER(writer);
			writer.write("Open file \"{}\" failure", __FILE__);
			throw std::runtime_error(writer.c_str());
		}
		catch (const std::runtime_error& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex.what());
			}
		}
	}
}


void BM_exception_str_lights_Exception(benchmark::State& state)
{
	lights::NullSink null;
	int perform_what = state.range(0);
	while (state.KeepRunning())
	{
		try
		{
			LIGHTS_THROW_EXCEPTION(lights::OpenFileError, __FILE__);
		}
		catch (const lights::Exception& ex)
		{
			if (perform_what)
			{
				ex.dump_message(null);
			}
		}
	}
}


// ------------------ Padding exception message. ----------------------

void BM_exception_pad_lights_ExceptionBinary(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			lights::benchmark::ExceptionBinary ex(EXCEPTION_FORMAT_STR);
			ex.add_arg(__LINE__, padding_str);
			throw ex;
		}
		catch (const lights::benchmark::ExceptionBinary& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex.what());
			}
		}
	}
}


void BM_exception_pad_lights_ExceptionBinary_ptr(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			auto ex = std::make_shared<lights::benchmark::ExceptionBinary>(EXCEPTION_FORMAT_STR);
			ex->add_arg(__LINE__, padding_str);
			throw ex;
		}
		catch (const std::shared_ptr<lights::benchmark::ExceptionBinary>& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex->what());
			}
		}
	}
}


void BM_exception_pad_lights_ExceptionText(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			lights::benchmark::ExceptionText ex(EXCEPTION_FORMAT_STR);
			ex.add_arg(__LINE__, padding_str);
			throw ex;
		}
		catch (const lights::benchmark::ExceptionText& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex.what());
			}
		}
	}
}


void BM_exception_pad_lights_ExceptionText_ptr(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			auto ex = std::make_shared<lights::benchmark::ExceptionText>(EXCEPTION_FORMAT_STR);
			ex->add_arg(__LINE__, padding_str);
			throw ex;
		}
		catch (const std::shared_ptr<lights::benchmark::ExceptionText>& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex->what());
			}
		}
	}
}


void BM_exception_pad_std_exception(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			LIGHTS_DEFAULT_TEXT_WRITER(writer)
			writer.write(EXCEPTION_FORMAT_ARGS);
			throw std::runtime_error(writer.c_str());
		}
		catch (const std::runtime_error& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex.what());
			}
		}
	}
}


void BM_exception_pad_std_exception_ptr(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			LIGHTS_DEFAULT_TEXT_WRITER(writer)
			writer.write(EXCEPTION_FORMAT_ARGS);
			throw std::make_shared<std::runtime_error>(writer.c_str());
		}
		catch (const std::shared_ptr<std::runtime_error>& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex->what());
			}
		}
	}
}


void BM_exception_pad_std_exception_format(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			throw std::runtime_error(lights::format(EXCEPTION_FORMAT_ARGS));
		}
		catch (const std::runtime_error& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(ex.what());
			}
		}
	}
}


void BM_exception_pad_boost_exception(benchmark::State& state)
{
	int perform_what = state.range(0);
	auto str_len = static_cast<std::size_t>(state.range(1));
	std::string padding_str(str_len, '1');

	while (state.KeepRunning())
	{
		try
		{
			LIGHTS_DEFAULT_TEXT_WRITER(writer)
			writer.write(EXCEPTION_FORMAT_ARGS);
			BOOST_THROW_EXCEPTION(std::runtime_error(writer.c_str()));
		}
		catch (const boost::exception& ex)
		{
			if (perform_what)
			{
				benchmark::DoNotOptimize(boost::diagnostic_information(ex));
			}
		}
	}
}


void custom_arguments(benchmark::internal::Benchmark* benchmark)
{
	for (int i = 1; i <= 1000; i *= 10)
	{
		benchmark->Args({0, i});
	}
//	benchmark->Args({0, 1});
}

void custom_arguments_show_what(benchmark::internal::Benchmark* benchmark)
{
	for (int i = 1; i <= 1000; i *= 10)
	{
		benchmark->Args({1, i});
	}
//	benchmark->Args({1, 1});
}


void BM_exception()
{
#define EXCEPTION_BENCHMARK(func) BENCHMARK(func)->Arg(0)->Arg(1)
	EXCEPTION_BENCHMARK(BM_exception_int);
	EXCEPTION_BENCHMARK(BM_exception_std_error_code);
	EXCEPTION_BENCHMARK(BM_exception_lights_Exception);
	EXCEPTION_BENCHMARK(BM_exception_str_std_exception);
	EXCEPTION_BENCHMARK(BM_exception_str_lights_Exception);

#define EXCEPTION_PAD_BENCHMARK(func) \
	BENCHMARK(func)->Apply(custom_arguments)->Apply(custom_arguments_show_what)

	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_lights_ExceptionBinary);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_lights_ExceptionBinary_ptr);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_lights_ExceptionText);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_lights_ExceptionText_ptr);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_std_exception);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_std_exception_ptr);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_std_exception_format);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_boost_exception);
	EXCEPTION_PAD_BENCHMARK(BM_exception_pad_boost_exception);
}

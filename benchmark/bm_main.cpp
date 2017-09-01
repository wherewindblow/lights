/**
 * bm_main.cpp
 * @author wherewindblow
 * @date   Aug 23, 2017
 */

#include <benchmark/benchmark.h>

#include "bm_format.h"
#include "bm_exception.h"
#include "bm_time.h"
#include "bm_logger.h"


int main(int argc, char** argv)
{
	// Init which type benchmark will be perform.
	BM_format();
	BM_logger();
//	BM_exception();
//	BM_time();

	// Start benchmark.
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
		return 1;
    ::benchmark::RunSpecifiedBenchmarks();
 }

# Benchmark is depend on some thrid part library.
# They are google benchmark, fmt and spdlog libraray.

include_directories(..)

set(LIGHTS_BENCHMARK_SRC
        bm_main.cpp
        bm_format.h bm_format.cpp
        bm_exception.h bm_exception.cpp
        bm_time.h bm_time.cpp
        bm_logger.h bm_logger.cpp)

add_executable(lights_benchmark ${LIGHTS_BENCHMARK_SRC})
target_link_libraries(lights_benchmark pthread benchmark fmt lights_static)

include_directories(..)

set(LIGHTS_BINARY_LOG_READER_SRC binary_log_reader.cpp)
add_executable(lights_bin_log_reader ${LIGHTS_BINARY_LOG_READER_SRC})
target_link_libraries(lights_bin_log_reader lights_static)

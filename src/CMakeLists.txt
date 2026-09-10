cmake_minimum_required(VERSION 3.10)
project(safealloca C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

enable_testing()

add_executable(safealloca_tests test.c)

target_include_directories(safealloca_tests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME safealloca_tests COMMAND safealloca_tests)

#
# Automagically set-up the coverage analysis with gcov :
# 	compilation flags on Coverage build type
#
# Usage :
# In the project CMakeLists.txt, adds the following lines :
#
# 	include (SetUpCoverage)
# 	SetUpCoverage ()
#
# Then you can add a custom command using the following variables :
# GCOV_PATH : path to gcov binary
# COVERAGE_REPORT_DIR : default result directory, set as ${PROJECT_BINARY_DIR}/coverage-reports/
# GCOV_RESULT_FILE : default report file, set as ${COVERAGE_REPORT_DIR}/gcov-result.xml
#
# eg. (from LP-Scheduler) :
# 	if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
# 		add_custom_command (TARGET ${UNITTEST_NAME}
# 			POST_BUILD
# 			COMMAND ${GCOVR_PATH} --root=${PROJECT_SOURCE_DIR} --object-directory=${PROJECT_BINARY_DIR} --gcov-exclude=".*/test/.*Test.c*" --branches --xml-pretty --output ${GCOV_RESULT_FILE}
# 			COMMENT "Generate coverage reports"
# 		)
# 	endif ()
#
# The default detection for GCOVR_PATH and GCOVR_PATH can be disabled
# by setting the required variable
# as a CMake variable ('set (GCOVR_PATH /my/path/to/gcovr-x.y.z)', '-D GCOVR_PATH=/my/path/to/gcovr-x.y.z')
# or as an environment variable

function (SetUpCoverage)

	message("ACCESSED SETUPCOVERAGE")

	if (MSVC)
		return ()
	endif ()

	if (NOT GCOV_PATH)
		set(GCOV_PATH $ENV{GCOV_PATH})
	endif ()

	if (NOT GCOV_PATH)
		find_program (GCOV_PATH gcov)
	endif ()

	if (NOT EXISTS ${GCOV_PATH})
		message (FATAL_ERROR "gcov not found! Aborting...")
	endif ()

	if(NOT GCOVR_PATH)
		set(GCOVR_PATH $ENV{GCOVR_PATH})
	endif ()

	if(NOT GCOVR_PATH)
		find_program (GCOVR_PATH gcovr)
	endif ()

	if (NOT EXISTS ${GCOVR_PATH})
		message (FATAL_ERROR "gcovr not found! Aborting...")
	endif ()

	set (GCOV_PATH "${GCOV_PATH}" CACHE FILEPATH "gcov binary path")
	set (GCOVR_PATH "${GCOVR_PATH}" CACHE FILEPATH "gcovr binary path")

	set (CMAKE_CXX_FLAGS_COVERAGE "-g -O0 -Wall -fPIC -fprofile-arcs -ftest-coverage -fno-inline")
	set (CMAKE_EXE_LINKER_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage -fno-inline")

	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		link_libraries (gcov)
	else ()
		set (CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_COVERAGE} --coverage")
	endif ()

	set (COVERAGE_REPORT_DIR "${PROJECT_BINARY_DIR}/coverage-reports")
	set (COVERAGE_REPORT_DIR "${COVERAGE_REPORT_DIR}" PARENT_SCOPE)

	set (CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_COVERAGE}" PARENT_SCOPE)
	set (CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_COVERAGE}" PARENT_SCOPE)
	set (GCOV_RESULT_FILE "${COVERAGE_REPORT_DIR}/gcov-result.xml" PARENT_SCOPE)

	file (MAKE_DIRECTORY ${COVERAGE_REPORT_DIR})

endfunction ()

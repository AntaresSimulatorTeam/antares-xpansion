# - Find XPRESS library
# Find the native XPRESS includes and library
# This module defines :
#  XPRESS_INCLUDE_DIR, where to find xprs.h, etc.
#  XPRESS_LIBRARIES, libraries to link against to use XPRESS.
#  XPRESS_DEFINITIONS, compiler switches required for using XPRESS.
#  XPRESS_FOUND, If false, do not try to use XPRESS.
# This module reads hints about search locations from variables :
#  XPRESS_FIND_VERSION (usually by calling find_package with version number)
#  XPRESS_ROOT, preferred installation prefix
#  XPRESS_LIBRARY_PATH, preferred library prefix

# ===============================
# Gurobi default version 
# ===============================

if(NOT XPRESS_FIND_VERSION)
	set(XPRESS_FIND_VERSION "7.9")
endif()

# ===============================
# XPRESS_ROOT default path
# ===============================

if(MSVC)
	set(XPRESS_ROOT "C:/Users/hudsonadmin/.conan/data/xpress/${XPRESS_FIND_VERSION}/eurodecision/external-binaries/package/ca33edce272a279b24f87dc0d4cf5bbdcffbc187/" CACHE PATH "XPRESS root directory")
else()
	set(XPRESS_ROOT "/opt/xpressmp-${XPRESS_FIND_VERSION}" CACHE PATH "XPRESS root directory")
endif()

# ===============================
# XPRESS_INCLUDE_DIR
# ===============================

if(XPRESS_INCLUDE_DIR)
	if(NOT EXISTS "${XPRESS_INCLUDE_DIR}/xprs.h")
		message(FATAL_ERROR "Invalid XPRESS_INCLUDE_DIR path ${XPRESS_INCLUDE_DIR} : no xprs.h file found")
	endif()
else()
	find_path(XPRESS_INCLUDE_DIR "xprs.h" HINTS "${XPRESS_ROOT}/include")
endif()

# ===============================
# XPRESS VERSION 
# ===============================

find_file(XPRESS_CONFIG_FILE NAMES version.txt PATHS ${XPRESS_ROOT})
if(NOT EXISTS ${XPRESS_CONFIG_FILE})
	message(STATUS "XPRESS : unknown version (no version.txt file found in ${XPRESS_ROOT})")
else()
	set(XPRESS_VERSION_REGEX "([0-9]+[.][0-9]+)")
	
	file(STRINGS ${XPRESS_CONFIG_FILE} XPRESS_VERSION_TXT_CONTENTS REGEX "FICO Xpress ")
	if(${XPRESS_VERSION_TXT_CONTENTS} MATCHES "FICO Xpress +${XPRESS_VERSION_REGEX}")
		set(XPRESS_VERSION ${CMAKE_MATCH_1})
	endif()
	
	unset(XPRESS_VERSION_TXT_CONTENTS)
	unset(XPRESS_VERSION_REGEX)
	unset(XPRESS_CONFIG_FILE)
endif()

# ===============================
# SXPRESS_LIBRARIES
# ===============================

if(NOT XPRESS_LIBRARY_PATH)
	set(XPRESS_LIBRARY_PATH "${XPRESS_ROOT}/lib")
endif()
find_library(XPRESS_LIBRARY NAMES xprs HINTS ${XPRESS_LIBRARY_PATH})

# ===============================
# XPRESS final steps
# ===============================

set(XPRESS_DEFINITIONS "-DXPRESS")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Xpress
	REQUIRED_VARS XPRESS_VERSION XPRESS_INCLUDE_DIR XPRESS_LIBRARY XPRESS_DEFINITIONS
	VERSION_VAR XPRESS_VERSION
	)

set (XPRESS_LIBRARIES ${XPRESS_LIBRARY})

mark_as_advanced(XPRESS_VERSION XPRESS_INCLUDE_DIR XPRESS_LIBRARY XPRESS_LIBRARIES XPRESS_DEFINITIONS)

#message("XPRESS : XPRESS_VERSION ${XPRESS_VERSION}")
#message("XPRESS : XPRESS_INCLUDE_DIR ${XPRESS_INCLUDE_DIR}")
#message("XPRESS : XPRESS_LIBRARY ${XPRESS_LIBRARY}")
#message("XPRESS : XPRESS_LIBRARIES ${XPRESS_LIBRARIES}")
#message("XPRESS : XPRESS_DEFINITIONS ${XPRESS_DEFINITIONS}")
#message("XPRESS : XPRESS_FOUND ${XPRESS_FOUND}")
#message(FATAL_ERROR "XPRESS : Work in Progress")


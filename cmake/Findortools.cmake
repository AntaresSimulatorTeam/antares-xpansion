# - Find OR-tools library
# Find or-tools includes and library

# This cmake module is based on : https://github.com/jwdinius/ortools-with-cmake/blob/master/cmake/Findortools.cmake

# This module defines :
#  ORTOOLS_INCLUDE_DIRS, where to find header files for ORTOOLS and its dependencies
#  ORTOOLS_LIBRARY_PATH, preferred library prefix
#  ORTOOLS_LIBRARIES, libraries to link against to use ORTOOLS.
#  ORTOOLS_DEFINITIONS, compiler switches required for using ORTOOLS.
#  ORTOOLS_FOUND, If false, do not try to use ORTOOLS.


#TODO test on a windows platform
#TODO use Components option in find_package to select solver and add specific definitions

# ===============================
# WINDOWS
# ===============================
if(MSVC)
    if(NOT TARGET ortools::ortools)
        if(ORTOOLS_ROOT)
            message("looking for ortools using root hint " ${ORTOOLS_ROOT})
            find_library(ORTOOLS_LIBRARIES NAME ortools HINTS ${ORTOOLS_ROOT} PATH_SUFFIXES lib )
            find_path(ORTOOLS_INCLUDE_DIRS NAME zlib.h HINTS ${ORTOOLS_ROOT} PATH_SUFFIXES include)
            link_directories(${ORTOOLS_LIBRARIES}/..)
            message("lib ortools by hint " ${ORTOOLS_LIBRARIES})
            message("include ortools by hint " ${ORTOOLS_INCLUDE_DIRS})
        else()
            find_library(ORTOOLS_LIBRARIES NAME ortools PATH_SUFFIXES lib )
            find_path(ORTOOLS_INCLUDE_DIRS NAME zlib.h PATH_SUFFIXES include)
        endif()
        set(ORTOOLS_DEFINITIONS /DNOMINMAX -DUSE_CBC -DUSE_CLP -DUSE_BOP -DUSE_GLOP)


        add_library(ortools::ortools STATIC IMPORTED GLOBAL)
        add_library(ortools ALIAS ortools::ortools)
        set_target_properties(ortools::ortools PROPERTIES IMPORTED_LOCATION ${ORTOOLS_LIBRARIES} )
        target_include_directories(ortools::ortools INTERFACE ${ORTOOLS_INCLUDE_DIRS})
        target_link_libraries(ortools::ortools INTERFACE ${ORTOOLS_LIBRARIES})
        target_compile_options(ortools::ortools INTERFACE ${ORTOOLS_DEFINITIONS})

        include(FindPackageHandleStandardArgs)
        # handle the QUIETLY and REQUIRED arguments and set ortools to TRUE
        # if all listed variables are TRUE
        find_package_handle_standard_args(ortools_FOUND  REQUIRED_VARS
                                        ORTOOLS_LIBRARIES ORTOOLS_INCLUDE_DIRS)
    endif()

# ===============================
# UNIX
# ===============================
elseif(UNIX)
    if(NOT TARGET ortools::ortools)
        # ===============================
        # ORTOOLS_ROOT default path
        # ===============================
        if(CONAN_ORTOOLS_ROOT)
            set(ORTOOLS_ROOT ${CONAN_ORTOOLS_ROOT} CACHE PATH "ORTOOLS root directory")
        else()
            set(ORTOOLS_ROOT "/opt/or-tools/" CACHE PATH "ORTOOLS root directory")
        endif()
        message("ortools using root : " ${ORTOOLS_ROOT})

        # ===============================
        # ORTOOLS_INCLUDE_DIRS
        # ===============================
        find_path(ORTOOLS_INCLUDE_DIRS NAME ortools HINTS ${ORTOOLS_ROOT} PATH_SUFFIXES include/)

        if(ORTOOLS_INCLUDE_DIRS)
            if(NOT EXISTS ${ORTOOLS_INCLUDE_DIRS}/ortools/linear_solver/linear_solver.h)
                message(FATAL_ERROR "Invalid ORTOOLS_INCLUDE_DIRS path ${ORTOOLS_INCLUDE_DIR} : no linear_solver/linear_solver.h file found in " ${ORTOOLS_INCLUDE_DIRS})
            else()
                message(STATUS "ortools library found here : ${ORTOOLS_INCLUDE_DIRS}")
            endif()
        else()
            message(WARNING "Failed to find path to ORTOOLS include directory")
        endif()


        # ===============================
        # Dependencies
        # ===============================
        set(LIB_TO_FIND
            CbcSolver #CBC_LNK
            Cbc
            OsiCbc
            Cgl
            ClpSolver
            Clp
            OsiClp
            Osi
            CoinUtils
            absl_bad_any_cast_impl #ABSL_LNK
            absl_bad_optional_access
            absl_bad_variant_access
            absl_base
            absl_city
            absl_civil_time
            absl_debugging_internal
            absl_demangle_internal
            absl_dynamic_annotations
            absl_examine_stack
            absl_failure_signal_handler
            absl_graphcycles_internal
            absl_hash
            absl_hashtablez_sampler
            absl_int128
            absl_leak_check
            absl_malloc_internal
            absl_optional
            absl_raw_hash_set
            absl_spinlock_wait
            absl_stacktrace
            absl_str_format_internal
            absl_strings
            absl_strings_internal
            absl_symbolize
            absl_synchronization
            absl_throw_delegate
            absl_time
            absl_time_zone
            protobuf #protobuf
            glog #glog
            gflags #gflags
        )

        foreach(X ${LIB_TO_FIND})
            find_library(LIB_${X} NAME ${X} PATH_SUFFIXES lib/)
            message(STATUS "${X} lib found here : ${LIB_${X}}")
            set(ORTOOLS_LIBRARIES ${ORTOOLS_LIBRARIES} ${LIB_${X}})
        endforeach()

        # ===============================
        # ORTOOLS LIBRARY PATH
        # ===============================
        if(NOT ORTOOLS_LIBRARY_PATH)
            set(ORTOOLS_LIBRARY_PATH "${ORTOOLS_ROOT}/lib")
        endif()
        find_library(LIB_ortools NAME ortools HINTS ${ORTOOLS_LIBRARY_PATH} PATH_SUFFIXES lib/)
        message(STATUS "ortools lib found here : ${LIB_ortools}")
        set(ORTOOLS_LIBRARIES ${ORTOOLS_LIBRARIES} ${LIB_ortools})


        # ===============================
        # ORTOOLS final steps
        # ===============================

        # Definitions
        set(ORTOOLS_DEFINITIONS -DUSE_CBC -DUSE_CLP -DUSE_BOP -DUSE_GLOP)


        add_library(ortools INTERFACE)
        add_library(ortools::ortools ALIAS ortools)
        target_include_directories(ortools INTERFACE ${ORTOOLS_INCLUDE_DIRS})
        target_link_libraries(ortools INTERFACE ${ORTOOLS_LIBRARIES})
        target_compile_options(ortools INTERFACE ${ORTOOLS_DEFINITIONS})

        include(FindPackageHandleStandardArgs)
        # handle the QUIETLY and REQUIRED arguments and set ortools_found to TRUE
        # if all listed variables are TRUE
        find_package_handle_standard_args(ortools
            REQUIRED_VARS ORTOOLS_LIBRARIES ORTOOLS_INCLUDE_DIRS)
    endif()

# ===============================
# OTHER
# ===============================
else()
    message(FATAL_ERROR "Non supported platform")
endif()

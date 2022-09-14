#####################
##  Antares solver  ##
#####################
find_package(gflags QUIET)
find_package(Cbc QUIET)
find_package(Cgl QUIET)
find_package(Clp QUIET)
find_package(CoinUtils QUIET)
find_package(Osi QUIET)
find_package(glog QUIET)
if (
        NOT gflags_FOUND OR
        NOT Cbc_FOUND OR
        NOT Cgl_FOUND OR
        NOT Clp_FOUND OR
        NOT CoinUtils_FOUND OR
        NOT Osi_FOUND OR
        NOT glog_FOUND
)
    if (DOWNLOAD_DEPS)
        message(STATUS "Downloading antares deps asset")
        if (MSVC)
            set(PLATFORM_TYPE windows-latest-Release.zip)
        else()
            execute_process(COMMAND bash "-c" "lsb_release -d | awk -F\"\t\" '{print $2}'" OUTPUT_VARIABLE SYSTEM_FLAVOUR)
            string(FIND ${SYSTEM_FLAVOUR} "Ubuntu" UBUNTU_FOUND)
            if (UBUNTU_FOUND EQUAL -1)
                message(STATUS "Will download Antares solver CentOS assets")
                set(PLATFORM_TYPE centos7-Release.tar.gz)
            else()
                message(STATUS "Will download Antares solver Ubuntu assets")
                set(PLATFORM_TYPE ubuntu-20.04-Release.tar.gz)
            endif()
        endif()
        execute_process(
                COMMAND wget https://github.com/AntaresSimulatorTeam/antares-deps/releases/download/v${ANTARES_DEPS_VERSION}/rte-antares-deps-${PLATFORM_TYPE}
        )
        execute_process(COMMAND mkdir -p ${DEPS_INSTALL_DIR})
        execute_process(
                COMMAND tar -xvf rte-antares-deps-${PLATFORM_TYPE} -C ${DEPS_INSTALL_DIR} --strip-components=1
        )
        execute_process(
                COMMAND rm -rf rte-antares-deps-${PLATFORM_TYPE}
        )
    else()
        message(WARNING "One or more dependency is missing but DOWNLOAD_DEPS is OFF")
    endif()
endif()

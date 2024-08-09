vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Osi"
        REF "d2809dd1ab01eb5c766edce7cea2ca2c1c5ecd2a" #stable/0.108
        SHA512 0b15a823666f7d381dadf360b6443fd28e9ba1f2c4c157309e10318d0a836014ec2a37bce0f0efdd9960769d839352aca0ba60f862b9c7eeaa96fabdffb7e9ca
        HEAD_REF master
)
if (WIN32)
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()
vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Osi PACKAGE_NAME Osi)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

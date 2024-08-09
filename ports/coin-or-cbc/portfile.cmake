vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Cbc"
        REF "b01a53da39577380acdb84d8a3577911c5d91a13" #stable/2.10
        SHA512 98a0e75dc1ac8ead134391ebd442d9cead0b1182620bff4058eb59e6cc843eaa9d0f70b956d1d1202836e5b3820a95107bec9e2233354080ab0a3a56dea1b20a
        HEAD_REF master
)
if (WIN32)
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()
vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Cbc PACKAGE_NAME Cbc)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

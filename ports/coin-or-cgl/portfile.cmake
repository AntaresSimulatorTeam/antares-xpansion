vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Cgl"
        REF "59d95fba6605329d615d44ac7be0be2397210d5a" #stable/0.60
        SHA512 5667c59a632bf30f43dbb993b434d81599d6933ffff874bbee92f4850229b0e6b0c20deacabc31d30ffd5c7484d048df1835e4524e98162b838deb621e94a373
        HEAD_REF master
)
if (WIN32)
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()
vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Cgl PACKAGE_NAME Cgl)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

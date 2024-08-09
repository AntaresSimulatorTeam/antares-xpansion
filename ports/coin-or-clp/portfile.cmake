vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Clp"
        REF "914e0af16285ab6b0514947296213a0e67e80880" #stable/1.17
        SHA512 c6a90007dc3177bb37800ae5c5c632403437df3a9ee54dfecf433e4c8b2ea403047c179ffdeef33b3aeab00c7ad2d859f4f56cca4488502dca8866889d909f1f
        HEAD_REF master
)
if (WIN32)
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()
vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Clp PACKAGE_NAME Clp)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

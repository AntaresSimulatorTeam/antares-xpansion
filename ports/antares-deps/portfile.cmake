vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "AntaresSimulatorTeam/antares-deps"
        REF "e6c4202cd40d959b84b4da83535a048cd9835732"
        SHA512 0ac6200bc14141c0d75f00f21234223f4dfc3b9eb532df4e95b93d2d679e00fe0007e3c026980481c31bd930207d4f0ee9ce142c90ef9ef1ea4989f00fe23c4b
        HEAD_REF feature/install
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME antares-deps CONFIG_PATH cmake)

#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

#file(INSTALL "${SOURCE_PATH}/LICENSE.TXT" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

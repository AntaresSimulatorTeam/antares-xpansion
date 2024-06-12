vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/CoinUtils"
        REF "d92accd9a2aa4cffe84e8f9a5a71eea06fb1ba87"
        SHA512 c5ac431a8c8b4a076620ec57b6f7899d7d8e729e97fb1628cfa58dbd8313fd2252ccf63cf8797499e0f4f43b08b2974fbbbff8f2fc4f9621da973642fd0d4fbe
        HEAD_REF master
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}/src"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME CoinUtils CONFIG_PATH cmake)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

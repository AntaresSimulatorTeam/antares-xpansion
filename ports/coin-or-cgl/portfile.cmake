vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Cgl"
        REF "925ad49fdd958ddb51f3ecfd87d222b0ea8d26a8"
        SHA512 365c204272f6d20f881ce5e14cfa3c2c1d5b0aef1ff92a7f8a889c708b2d091105bc1ba2dfdee911b3cab876af677f5ee3e55657f9fdb9356915966379587261
        HEAD_REF master
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Cgl PACKAGE_NAME Cgl)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

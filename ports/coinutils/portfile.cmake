vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/CoinUtils"
        REF "675cbb8e131f07705544a2e9074355cfa1a319b4" #stable 2.11
        SHA512 19e68fd43a90f3c6ba84aef90fc8b108550efee6345a3a467995791955322b531abf6e410ed656b10a353108e73d8558e9cb6e24aa93a273686e226d17381c48
        HEAD_REF master
)

if (WIN32)
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/CoinUtils)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

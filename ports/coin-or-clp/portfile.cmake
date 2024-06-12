vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Clp"
        REF "d8cdeb5fd6d51ac7d0f50778d2b2feccaa716228"
        SHA512 58e737deeb5276e4894fbebac0f60da13e8419b6cff531be381bc582320ceeab0f4688c8ec29d4e7248b6cc5256edb27b2e98496913262ce0f2960b6b6ab598c
        HEAD_REF master
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Clp PACKAGE_NAME Clp)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

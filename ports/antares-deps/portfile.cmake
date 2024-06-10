vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "AntaresSimulatorTeam/antares-deps"
        REF "d1b03d7fcfc938c19164f3e6b8cb73ef73755350"
        SHA512 3f482c67e14f411fa9b27baab0c4c6be46061c81bb4f0df8f7b5d8ec6283eaf180c23111c9a5994e3be4f86e10631c0323f69eba6b303a6a5519826258144f9f
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

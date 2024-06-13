vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Osi"
        REF "d724c4a5a0d87669d63e44179c468d8a02f4f69a"
        SHA512 fb173c04e3920c863e4d29e202cbaf182ee21332dc05ce293ffc39ead1b124f86f9682cc12032d216a712ea9aa3d0c98176bcbcd1b217e93afc6ce5a1ced68ed
        HEAD_REF master
)
if(MSVC OR WIN32)
    message(WARNING "BUILDING FOR MSVC WITH SHARED LIBS")
    vcpkg_cmake_configure(
            SOURCE_PATH "${SOURCE_PATH}"
            OPTIONS -DDLL_EXPORT
    )
else ()
vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)
endif ()

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Osi PACKAGE_NAME Osi)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

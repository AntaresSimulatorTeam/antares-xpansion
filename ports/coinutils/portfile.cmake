vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/CoinUtils"
        REF "d92accd9a2aa4cffe84e8f9a5a71eea06fb1ba87"
        SHA512 addd95047c935fd3199f6951bf6f269887c0a66f285d4d9947b6290de24ab54867f459a340da1010451331272e4168d413ab167c52019fb4dbe486633f8b93d9
        HEAD_REF master
)

if(MSVC OR WIN32)
    message(WARNING "BUILDING FOR MSVC WITH SHARED LIBS")
    vcpkg_cmake_configure(
            SOURCE_PATH "${SOURCE_PATH}"
            OPTIONS
            "-DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_LIST_DIR}/static_patch.cmake"

    )
else ()
    vcpkg_cmake_configure(
            SOURCE_PATH "${SOURCE_PATH}"
    )
endif ()

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/CoinUtils)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

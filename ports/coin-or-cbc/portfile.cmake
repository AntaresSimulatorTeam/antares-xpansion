vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Cbc"
        REF "5714054827d852fae3beb7a4065f84ea56f207bb"
        SHA512 d9d563b7af6102ba29c522cabf983ebde31a38f7010f578bc0dca258e63245c92b975fd0a3d5b96c7eeba886034b1e188d1399eb0d67769e0fa560c23e088ebf
        HEAD_REF master
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

#vcpkg_cmake_config_fixup(PACKAGE_NAME CoinUtils CONFIG_PATH cmake)

#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

# Temporary overlay port for the BendersByBatch CLP crash investigation
# (PR #1267): identical to the antares-vcpkg-registry's coin-or-clp port,
# with a small diagnostic patch added (guarded behind CLP_DIAG_INSTRUMENTATION,
# only defined here) that logs the exact ClpSimplex/ClpNonLinearCost/createRim
# decisions relevant to the "stale nonLinearCost_ after AddRows on a
# warm-started solver" hypothesis. Not for permanent use - remove this
# directory and the VCPKG_OVERLAY_PORTS wiring in build_windows.yml once the
# crash is diagnosed.
vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO "Mizux/Clp"
        REF "14ab15c7dfc59b6bdb5455d42a2d5e9e1ec28a75" #1.17.10
        SHA512 8b7783daef891733b79c6d54e52cfd9f5f2114394edecfaa4b66290ffa17ae7e174487c625b2576205772f018bde10da4baba933eced0f053e521e734fbc40db
        HEAD_REF master
        PATCHES
                clp-diag-instrumentation.patch
)
if (VCPKG_TARGET_IS_WINDOWS)
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

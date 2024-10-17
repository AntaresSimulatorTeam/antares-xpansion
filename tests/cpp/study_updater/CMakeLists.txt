add_executable(study_updater_test)
target_sources(study_updater_test PRIVATE
        JsonXpansionReaderTest.cc
)

# Get source dir to include private headers
# Better way would be to rework test and use StudyUpdater::update as entry point
get_target_property(STUDY_UPDATER_SOURCE_DIR ${PROJECT_NAME}::xpansion_study_updater_lib SOURCE_DIR)

target_include_directories(study_updater_test
        PRIVATE
        ${STUDY_UPDATER_SOURCE_DIR}/private # for private headers
)

target_link_libraries(study_updater_test
        PRIVATE
        GTest::Main
        ${PROJECT_NAME}::xpansion_study_updater_lib
)
add_test(NAME study_updater_test COMMAND study_updater_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST study_updater_test PROPERTY LABELS unit)

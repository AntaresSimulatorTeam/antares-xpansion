add_library (xpansion_study_updater_lib 
		${CMAKE_CURRENT_SOURCE_DIR}/LinkCapacitiesCSVWriter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateStrategy.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateLinkCapacitiesStrategy.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateLinkParameterStrategy.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateRunner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkParametersCSVOverwriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdater.cpp
)

target_link_libraries (xpansion_study_updater_lib 
	PRIVATE  
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::lp_namer_model
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier
		antaresXpansion::helpers
		)
get_target_property(lp_namer_model_include lp_namer_model INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(helpers_include helpers INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(lp_namer_helper_include lp_namer_helper INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(lp_namer_input_reader_include lp_namer_input_reader INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(lp_namer_problem_modifier_include lp_namer_problem_modifier INTERFACE_INCLUDE_DIRECTORIES)
					

target_include_directories(xpansion_study_updater_lib 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${lp_namer_model_include}
		${helpers_include}
		${lp_namer_helper_include}
		${lp_namer_input_reader_include}
		${lp_namer_problem_modifier_include}
		)

add_library (${PROJECT_NAME}::xpansion_study_updater_lib ALIAS xpansion_study_updater_lib)
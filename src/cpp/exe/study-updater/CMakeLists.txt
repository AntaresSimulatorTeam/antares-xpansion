add_executable (xpansion-study-updater 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (xpansion-study-updater 
	PRIVATE  
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::lp_namer_model
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier	
		Boost::boost Boost::program_options)

install(TARGETS xpansion-study-updater DESTINATION bin)

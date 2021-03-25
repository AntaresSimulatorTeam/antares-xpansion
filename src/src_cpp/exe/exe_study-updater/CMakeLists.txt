add_executable (xpansion-study-updater 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (xpansion-study-updater 
	PRIVATE 
		${PROJECT_NAME}::lpnamer		
		Boost::boost Boost::program_options)

install(TARGETS xpansion-study-updater DESTINATION bin)

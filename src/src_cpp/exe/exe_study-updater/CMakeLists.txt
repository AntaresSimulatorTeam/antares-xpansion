add_executable (xpansion-study-updater 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

target_link_libraries(xpansion-study-updater
PUBLIC
stdc++fs
)

endif()

target_link_libraries (xpansion-study-updater 
	PRIVATE 
		${PROJECT_NAME}::lpnamer		
		Boost::boost Boost::program_options)

install(TARGETS xpansion-study-updater DESTINATION bin)

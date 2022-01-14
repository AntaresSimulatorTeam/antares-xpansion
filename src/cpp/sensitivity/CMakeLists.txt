

add_library (sensitivity
        SensitivityAnalysis.cpp SensitivityLogger.cpp SensitivityWriter.cpp)

target_include_directories (sensitivity
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_link_libraries (sensitivity
        PUBLIC
        solvers
)

# target name to generate here
set(TARGET lfd_rendering)

# glob source files
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_executable(${TARGET} ${SOURCES} ${HEADERS})

# forces executables to be compiled to src folder, to allow relative paths for shaders
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")



# ----------------------------------------------------------
# dependencies
if(WIN32)
	add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../external/cppgl/external/thirdparty/win/bin/x64" $<TARGET_FILE_DIR:${TARGET}>)
	add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/external/cppgl/cppgl.dll" $<TARGET_FILE_DIR:${TARGET}>)
endif()

# built libs
target_link_libraries(${TARGET} cppgl)

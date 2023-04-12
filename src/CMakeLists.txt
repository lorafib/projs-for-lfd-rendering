# glob for source files
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_library(cppgl SHARED ${SOURCES})
generate_export_header(cppgl)

# public interface (cppgl.h is an available header file for all dependant projects)
target_include_directories(cppgl INTERFACE .)

# ----------------------------------------------------------
# dependencies

# opengl
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_include_directories(cppgl PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(cppgl ${OPENGL_LIBRARIES})

if(UNIX)
    target_link_libraries(cppgl stdc++fs) # required for std::filesystem
else()
    target_compile_definitions(cppgl PRIVATE -DBUILD_CPPGL_DLL)
    target_compile_definitions(cppgl PRIVATE -DBUILD_SHARED_LIBS)
endif()

target_link_libraries(cppgl libglew_shared glfw assimp)

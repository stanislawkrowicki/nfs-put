cmake_minimum_required(VERSION 3.16)
project(nfsput)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
)
FetchContent_MakeAvailable(glm)

add_executable(nfsput main.cpp)

target_link_libraries(nfsput PRIVATE glfw)
target_include_directories(nfsput PRIVATE ${glm_SOURCE_DIR})

if(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(nfsput PRIVATE OpenGL::GL)
endif()

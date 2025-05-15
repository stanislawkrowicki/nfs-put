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

# Assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.4.3
)
FetchContent_MakeAvailable(assimp)

# stb_image
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)

FetchContent_MakeAvailable(stb)

add_executable(nfsput main.cpp shader.cpp
        camera.hpp
        model.hpp
        model.cpp)

target_link_libraries(nfsput PRIVATE glfw assimp)
target_include_directories(nfsput PRIVATE ${glm_SOURCE_DIR} ${stb_SOURCE_DIR} .)

if(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    target_link_libraries(nfsput PRIVATE OpenGL::GL GLEW::GLEW)
endif()

set(MODELS_DIR "${CMAKE_SOURCE_DIR}/models/")
set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders/")
add_definitions(-DMODELS_PATH="${MODELS_DIR}")
add_definitions(-DSHADERS_PATH="${SHADERS_DIR}")
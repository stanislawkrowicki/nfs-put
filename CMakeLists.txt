cmake_minimum_required(VERSION 3.16)

# OpenGL target for Bullet
set(OpenGL_GL_PREFERENCE GLVND)

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

# Bullet
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        bullet
        GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
        GIT_TAG master
)

FetchContent_MakeAvailable(bullet)

add_executable(nfsput main.cpp shader.cpp
        camera.hpp
        model.hpp
        model.cpp
        camera.cpp
        physics_debug.hpp
        physics_debug.cpp
        physics.hpp
        physics.cpp
        vehicle.hpp
        vehicle.cpp
        vehicle_config.hpp
        vehicle_manager.hpp
        skybox.hpp
)

target_link_libraries(nfsput PRIVATE glfw assimp BulletDynamics BulletCollision LinearMath)
target_include_directories(nfsput PRIVATE ${glm_SOURCE_DIR} ${stb_SOURCE_DIR} ${bullet_SOURCE_DIR}/src .)

if(WIN32)
    # OpenGL
    target_link_libraries(nfsput PRIVATE opengl32 gdi32)

    # GLEW
    set(GLEW_INCLUDE_DIR "C:/Users/marha/Documents/studia/semestr 4/grafika/projekt/glew-2.1.0/include")
    set(GLEW_LIBRARY "C:/Users/marha/Documents/studia/semestr 4/grafika/projekt/glew-2.1.0/lib/Release/x64/glew32.lib")

    target_include_directories(nfsput PRIVATE ${GLEW_INCLUDE_DIR})
    target_link_libraries(nfsput PRIVATE ${GLEW_LIBRARY})
endif()
set(MODELS_DIR "${CMAKE_SOURCE_DIR}/models/")
set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders/")
set(SKYBOX_DIR "${CMAKE_SOURCE_DIR}/skybox/")
add_definitions(-DMODELS_PATH="${MODELS_DIR}")
add_definitions(-DSHADERS_PATH="${SHADERS_DIR}")
add_definitions(-DSKYBOX_PATH="${SKYBOX_DIR}")
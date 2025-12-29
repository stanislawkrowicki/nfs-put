cmake_minimum_required(VERSION 3.16)

# OpenGL target for Bullet
set(OpenGL_GL_PREFERENCE GLVND)

project(nfsput)

set(CMAKE_CXX_STANDARD 20)

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

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)


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
        opponent.hpp
        opponent.cpp
        opponent_path.hpp
        opponent_path.cpp
        netcode/client/udp_client.cpp
        netcode/client/udp_client.hpp
        netcode/client/tcp_client.cpp
        netcode/client/tcp_client.hpp
        netcode/shared/packets/udp/udp_packet.hpp
        netcode/shared/packets/udp/client/state_packet.hpp
        netcode/shared/packets/udp/udp_packet_header.hpp
        netcode/shared/crc32.hpp
        netcode/shared/crc32.cpp
        netcode/shared/deserialization_error.hpp
        netcode/shared/packets/udp/udp_packet_type.hpp
        netcode/shared/packets/udp/server/opponent_states_packet.hpp
        netcode/shared/client_state.hpp
        netcode/client/handlers/opponent_states_handler.hpp
        default_vehicle_model.hpp
        netcode/shared/utils/byte_dump.hpp
        netcode/client/opponent_manager.cpp
        netcode/client/opponent_manager.hpp
        netcode/shared/client_inputs.hpp
        netcode/shared/packets/tcp/tcp_packet_type.hpp
        netcode/shared/packets/tcp/tcp_packet.hpp
        netcode/shared/packets/tcp/tcp_packet_header.hpp
        netcode/shared/packets/tcp/server/race_start_packet.hpp
        netcode/shared/packets/tcp/server/provide_name_packet.hpp
        netcode/shared/packets/tcp/client/name_packet.hpp
        netcode/client/handlers/provide_name_handler.hpp
        netcode/shared/packets/tcp/server/name_taken_packet.hpp
        netcode/shared/packets/tcp/server/name_accepted_packet.hpp
        netcode/client/handlers/name_taken_handler.hpp
        netcode/client/handlers/name_accepted_handler.hpp
)

target_link_libraries(nfsput PRIVATE glfw assimp BulletDynamics BulletCollision LinearMath nlohmann_json::nlohmann_json)
target_include_directories(nfsput PRIVATE ${glm_SOURCE_DIR} ${stb_SOURCE_DIR} ${bullet_SOURCE_DIR}/src .)

if (UNIX)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    target_link_libraries(nfsput PRIVATE OpenGL::GL GLEW::GLEW)

elseif (WIN32)
    # OpenGL
    target_link_libraries(nfsput PRIVATE opengl32 gdi32)

    # GLEW
    set(GLEW_INCLUDE_DIR "glew-2.1.0/include")
    set(GLEW_LIBRARY "glew-2.1.0/lib/Release/x64/glew32.lib")

    target_include_directories(nfsput PRIVATE ${GLEW_INCLUDE_DIR})
    target_link_libraries(nfsput PRIVATE ${GLEW_LIBRARY})
endif()

set(MODELS_DIR "${CMAKE_SOURCE_DIR}/models/")
set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders/")
set(SKYBOX_DIR "${CMAKE_SOURCE_DIR}/skybox/")
add_definitions(-DMODELS_PATH="${MODELS_DIR}")
add_definitions(-DSHADERS_PATH="${SHADERS_DIR}")
add_definitions(-DSKYBOX_PATH="${SKYBOX_DIR}")
add_definitions(-DSOURCE_PATH="${CMAKE_SOURCE_DIR}")

add_subdirectory(netcode/server)
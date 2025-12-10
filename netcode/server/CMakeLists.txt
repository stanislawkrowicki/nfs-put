cmake_minimum_required(VERSION 3.16)

project(server)

add_executable(server
        main.cpp
        connection_manager.hpp
        client_handle.hpp
        connection_manager.cpp
        bsd_server.cpp
        bsd_server.hpp
        udp_server.cpp
        udp_server.hpp
        client_manager.hpp
        loop.cpp
        loop.hpp
        ../shared/packets/udp/udp_packet.hpp
        ../shared/packets/udp/client/position_packet.hpp
        ../shared/packets/udp/packet_header.hpp
        ../shared/crc32.hpp
        ../shared/crc32.cpp
        ../shared/deserialization_error.hpp
        ../shared/packets/udp/udp_packet_type.hpp
        handlers/position_handler.hpp
        ../shared/packets/udp/server/opponent_positions_packet.hpp
        ../shared/player_position.hpp)

target_link_libraries(server PRIVATE LinearMath)

target_include_directories(server PRIVATE ${bullet_SOURCE_DIR}/src)
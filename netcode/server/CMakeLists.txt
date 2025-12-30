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
        tcp_server.cpp
        tcp_server.hpp
        client_manager.hpp
        loop.cpp
        loop.hpp
        ../shared/packets/udp/udp_packet.hpp
        ../shared/packets/udp/client/state_packet.hpp
        ../shared/packets/udp/udp_packet_header.hpp
        ../shared/crc32.hpp
        ../shared/crc32.cpp
        ../shared/deserialization_error.hpp
        ../shared/packets/udp/udp_packet_type.hpp
        handlers/state_handler.hpp
        ../shared/packets/udp/server/opponent_states_packet.hpp
        ../shared/client_state.hpp
        ../shared/utils/byte_dump.hpp
        ../shared/packets/tcp/tcp_packet_type.hpp
        ../shared/packets/tcp/tcp_packet.hpp
        ../shared/packets/tcp/tcp_packet_header.hpp
        ../shared/packets/tcp/server/race_start_packet.hpp
        ../shared/packets/tcp/server/provide_name_packet.hpp
        ../shared/packets/tcp/client/name_packet.hpp
        handlers/name_handler.hpp
        ../shared/packets/tcp/server/name_taken_packet.hpp
        ../shared/packets/tcp/server/name_accepted_packet.hpp
        handlers/udp_info_handler.hpp)

target_link_libraries(server PRIVATE LinearMath)

target_include_directories(server PRIVATE ${bullet_SOURCE_DIR}/src)
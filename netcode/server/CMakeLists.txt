cmake_minimum_required(VERSION 3.16)

add_executable(server
        main.cpp
        connection_manager.hpp
        client_handle.hpp
        connection_manager.cpp
        bsd_server.cpp
        bsd_server.hpp
        udp_server.cpp
        udp_server.hpp
        client_manager.hpp)

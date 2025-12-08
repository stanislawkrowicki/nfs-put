cmake_minimum_required(VERSION 3.16)

add_executable(server
        main.cpp
        client_handle.hpp
        bsd_server.cpp
        bsd_server.hpp
        udp_server.cpp
        udp_server.hpp
        client_manager.hpp
        tcp_server.cpp
        tcp_server.hpp)

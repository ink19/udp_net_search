cmake_minimum_required(VERSION 3.4)

project(udp_net_search LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED program_options)

add_executable(MainServ main_serv.cpp)

target_link_libraries(MainServ Boost::program_options)
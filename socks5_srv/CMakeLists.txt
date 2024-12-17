cmake_minimum_required(VERSION 3.5)

project(socksv5_pkg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

set( PKG_SOURCES
    main.cpp
    socks_server.cpp
    socks_client.cpp)

add_executable(${PROJECT_NAME} ${PKG_SOURCES})

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

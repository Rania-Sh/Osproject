cmake_minimum_required(VERSION 3.20)
project(osproject C)

set(CMAKE_C_STANDARD 11)

add_executable(osproject
        main.c
        animation.c
        animationui.c
        dijkstra.c
)


find_package(raylib REQUIRED)

target_link_libraries(osproject raylib m pthread dl rt X11 GL)


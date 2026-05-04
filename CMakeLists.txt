cmake_minimum_required(VERSION 3.20)
project(osproject C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(osproject
        main.c
        animation.c
        animationui.c
        dijkstra.c
)

target_link_libraries(osproject raylib)

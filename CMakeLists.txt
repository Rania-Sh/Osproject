cmake_minimum_required(VERSION 3.20)
project(osproject C)

set(CMAKE_C_STANDARD 11)

add_executable(osproject main.c
        animation.h
        animation.c
        animationui.h
        animationui.c
)
#Link raylib
find_package(raylib REQUIRED)
target_link_libraries(osproject raylib m pthread dl)


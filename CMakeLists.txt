cmake_minimum_required(VERSION 3.16)

project(Obj_Renderer)

find_package(X11 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Obj_Renderer
    src/main.cpp
)

target_include_directories(Obj_Renderer PRIVATE ${X11_INCLUDE_DIR})
target_link_libraries(Obj_Renderer PRIVATE ${X11_LIBRARIES})

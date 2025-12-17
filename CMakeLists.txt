cmake_minimum_required(VERSION 3.5)

project(Criminisi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Imagine REQUIRED)

include_directories(src)

add_executable(Criminisi 
    src/main.cpp
    src/patch.cpp
    src/image.cpp
    src/image.h
    src/patch.h
    src/polygons.cpp
    src/polygons.h
)

target_link_libraries(Criminisi Imagine::Graphics Imagine::Images)

include(GNUInstallDirs)
install(TARGETS Criminisi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
cmake_minimum_required(VERSION 3.5)

project(Criminisi LANGUAGES CXX)

find_package(Imagine REQUIRED COMPONENTS Graphics)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Criminisi main.cpp
    patch.cpp
    image.cpp
    image.h
    patch.h
    polygons.cpp
    polygons.h
)

target_link_libraries(Criminisi Imagine::Graphics ${OpenCV_LIBS})

include(GNUInstallDirs)
install(TARGETS Criminisi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 23)

# Project information
project(Unreal)
set(SOURCES
    main.cpp
    format.h
    formats/zip.h
    formats/zip.cpp
    formats/umod.cpp
    formats/umod.h
    formats/rar.h
    formats/rar.cpp
    modfile.h
)

# Dependencies
find_package(libzip REQUIRED)
set(BUILD_LINK_LIBS
    zip unarr
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${BUILD_LINK_LIBS})

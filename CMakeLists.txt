cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 23)

# Project information
project(UTModLoader VERSION 0.0.0 LANGUAGES CXX)
set(SOURCES
    src/main.cpp
    src/modfile.h
    src/formats/zip.h
    src/formats/zip.cpp
    src/formats/umod.cpp
    src/formats/umod.h
    src/formats/rar.h
    src/formats/rar.cpp
    src/commands/extract.cpp
    src/commands/extract.h
    src/util/error.h
    src/util/container.h
    src/util/str.h
)

configure_file(src/config/config.h.in config.h)
include_directories(${PROJECT_BINARY_DIR})

# Dependencies
find_package(libzip REQUIRED)
set(BUILD_LINK_LIBS
    zip unarr
)

# On windows, search MINGW for include paths
if (WIN32)
    # NOTE: I installed libzip using the mingw-w64-x86_64-libzip package, but
    # libunarr was installed using the AUR (unmodified PKGBUILD), so we need to
    # include it's files from MSYS_PFX
    # todo: can this be made a little easier ?

    set(MSYS_PFX "C:/msys64/msys64/usr")
    set(MINGW_PFX "C:/msys64/mingw64")
    set(CMAKE_PREFIX_PATH "${MINGW_PFX}/include/;${MSYS_PFX}/include/")
    link_directories("${MSYS_PFX}/lib/")
    link_directories("${MINGW_PFX}/lib/")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${BUILD_LINK_LIBS})
target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Werror -Wfatal-errors)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-g -Wpedantic -pedantic-errors>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-O3>)

# Post-build configuration
if (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_PREFIX_PATH})

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${MSYS_PFX}/bin/libunarr.dll" "${CMAKE_BINARY_DIR}/libunarr.dll"
    )
endif()

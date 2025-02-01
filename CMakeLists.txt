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
if (WIN32)
    set(UNRAR_LIB "unrar64")
else()
    set(UNRAR_LIB "unrar")
endif()

set(BUILD_LINK_LIBS
    zip ${UNRAR_LIB}
)

# On Windows, search MINGW for include paths
if (WIN32)
    set(MINGW_PFX "C:/msys64/mingw64")
    set(WIN_PFX "C:/Program Files\ (x86)/UnrarDLL")     # UnRAR is installed outside of MinGW
    set(CMAKE_PREFIX_PATH "${MINGW_PFX}/include/;${WIN_PFX}/;${WIN_PFX}/x64/")
    link_directories("${WIN_PFX}/x64/")
    link_directories("${MINGW_PFX}/lib/")
endif()

# Build configuration
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${BUILD_LINK_LIBS})
target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Werror -Wfatal-errors)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-g -Wpedantic -pedantic-errors>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-O3>)

# Post-build configuration
if (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_PREFIX_PATH})

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${MINGW_PFX}/bin/libzip.dll" "${CMAKE_BINARY_DIR}/libzip.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${WIN_PFX}/x64/UnRAR64.dll" "${CMAKE_BINARY_DIR}/UnRAR64.dll"
    )
endif()

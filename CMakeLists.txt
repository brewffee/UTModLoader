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
    macros.h
)

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

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${BUILD_LINK_LIBS})

# Post-build configuration
if (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_PREFIX_PATH})

    add_custom_command(TARGET Unreal PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${MSYS_PFX}/bin/libunarr.dll" "${CMAKE_BINARY_DIR}/libunarr.dll"
    )
endif()

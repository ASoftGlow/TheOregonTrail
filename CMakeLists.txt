cmake_minimum_required(VERSION 3.13)
set(CMAKE_SUPPRESS_REGENERATION true)
project(TheOregonTrail C)

option(TTY "Build assuming TTY" True)
option(ASCII "Use only ASCII instead of UTF-8" False)
option(MUTE "Don't use sound" True)
option(DISCORD "Enable Discord rich presence support" False)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if(APPLE)
    message(FATAL_ERROR "L")
endif()

if(ASCII)
    add_definitions(-DTOT_ASCII)
endif()

if(MUTE)
    add_definitions(-DTOT_MUTE)
endif()

if(TTY)
    add_definitions(-DTOT_TTY)
endif()

if(DISCORD)
    add_definitions(-DTOT_DISCORD)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
endif()

if(WIN32)
    include(generate_product_version)
    generate_product_version(
       VersionFilesOutputVariable
       NAME ${CMAKE_PROJECT_NAME}
       ICON "${PROJECT_SOURCE_DIR}/resources/tot.ico"
       COMPANY_NAME "ASoftGlow Software"
       FILE_DESCRIPTION "A TUI game heavily based on MECC's 1990 The Oregon Trail"
    )
endif()


add_subdirectory(src)
set_property(TARGET tot PROPERTY C_STANDARD 11)
set_property(TARGET tot PROPERTY C_STANDARD_REQUIRED ON)
target_include_directories(
    tot PUBLIC "${PROJECT_SOURCE_DIR}/include"
    tot PUBLIC lib
    tot PUBLIC "${PROJECT_BINARY_DIR}/generated"
)

set(BUILD_SHARED_LIBS OFF)

if(NOT TTY)
    set(NFD_PORTAL ON)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/nativefiledialog-extended)
    target_link_libraries(tot PRIVATE nfd)
endif()

if(NOT MUTE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/portaudio)
    target_link_libraries(tot PRIVATE PortAudio)

    set(BUILD_TESTING OFF)
    set(BUILD_EXAMPLES OFF)
    set(ENABLE_CPACK OFF)
    set(INSTALL_MANPAGES OFF)
    set(ENABLE_PACKAGE_CONFIG OFF)
    set(INSTALL_PKGCONFIG_MODULE OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libsndfile)
    target_link_libraries(tot PRIVATE sndfile)
endif()

if(DISCORD)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_EXAMPLES OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/discord-rpc)
    target_link_libraries(tot PRIVATE discord-rpc stdc++)
endif()


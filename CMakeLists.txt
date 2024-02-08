cmake_minimum_required(VERSION 3.13)
set(CMAKE_SUPPRESS_REGENERATION true)
project(TheOregonTrail C)

option(TTY "Build assuming TTY" False)
option(ASCII "Use only ASCII instead of UTF-8" False)

if(ASCII)
    add_definitions(-DTOT_ASCII)
endif()

if(TTY)
    add_definitions(-DTOT_TTY)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
endif()

add_subdirectory(src)
set_property(TARGET tot PROPERTY C_STANDARD 11)
set_property(TARGET tot PROPERTY C_STANDARD_REQUIRED ON)
target_include_directories(tot PUBLIC lib)
target_include_directories(tot PUBLIC "${PROJECT_SOURCE_DIR}/include"
)

if (TTY)
else()
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/nativefiledialog-extended)
    target_link_libraries(tot PRIVATE nfd)
endif()

#add_subdirectory(${PROJECT_SOURCE_DIR}/lib/CSFML)
#target_link_libraries(tot PRIVATE CSFML)
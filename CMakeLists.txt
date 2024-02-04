cmake_minimum_required(VERSION 3.13)
set(CMAKE_SUPPRESS_REGENERATION true)
project(TheOregonTrail C)

option(TOT_TTY "Build assuming TTY" False)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(src)
set_property(TARGET tot PROPERTY C_STANDARD 11)
set_property(TARGET tot PROPERTY C_STANDARD_REQUIRED ON)
target_include_directories(tot PUBLIC lib)
target_include_directories(tot PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(tot "${PROJECT_SOURCE_DIR}/lib/csfml-audio.lib"
)

if (NOT ${TOT_TTY})
  if (WIN32)
  target_link_libraries(tot "${PROJECT_SOURCE_DIR}/lib/nfd.lib")
  elseif (UNIX)
  target_link_libraries(tot "${PROJECT_SOURCE_DIR}/lib/libnfd.a")
  endif (WIN32)
endif()
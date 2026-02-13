SET(SOURCE_FILES
	base.c
  ansi_codes.c
	input.c
	main.c
	settings.c
	state.c
	static.c
	store.c
	tui.c
  formatted_lines.c
	utils.c
	blackjack/game.c
	blackjack/card.c
  screens/learn.c
  screens/main_settings.c
  screens/prepare.c
  screens/trail.c
  screens/map.c
  screens/general_store.c

  ../test/test.c
  ../test/wrapping.c
)

if(WIN32)
	list(APPEND SOURCE_FILES ${VersionFilesOutputVariable})
endif()

if(NOT MUTE)
	list(APPEND SOURCE_FILES music.c)
endif()

if(DISCORD)
	list(APPEND SOURCE_FILES discord.c)
endif()

# this should be cleaned up later
file(WRITE "${CMAKE_BINARY_DIR}/licenses.c" "#include \"licenses.h\"\nconst SoftwareLicense LICENSES[] = {\n")
macro(add_license name path clean)
  file(READ "${CMAKE_SOURCE_DIR}/lib/${path}" filedata)
  # this sanitization is not perfect but good enough
  string(REPLACE \" \\\" filedata ${filedata})
  if(${clean})
    # remove single newlines
    string(REPLACE \n\n {tmp} filedata ${filedata})
    string(REPLACE \n \  filedata ${filedata})
    string(REPLACE {tmp} \\n\\n filedata ${filedata})
  else()
    string(REPLACE \n \\n filedata ${filedata})
  endif()
  file(APPEND "${CMAKE_BINARY_DIR}/licenses.c" "{\"${name}\", \"${filedata}\"},\n")
	list(APPEND SOURCE_FILES "${CMAKE_BINARY_DIR}/licenses.c")
endmacro()

add_license("Discord RPC" discord-rpc/LICENSE ON)
add_license("cfgpath.h" cfgpath/LICENSE ON)
add_license("Native File Dialog Extended" nativefiledialog-extended/LICENSE OFF)
add_license("PortAudio" portaudio/LICENSE.txt OFF)

file(APPEND "${CMAKE_BINARY_DIR}/licenses.c" "};\nconst unsigned LICENSES_COUNT = sizeof(LICENSES)/sizeof(*LICENSES);\n")

add_executable(tot ${SOURCE_FILES})
set_target_properties(tot PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

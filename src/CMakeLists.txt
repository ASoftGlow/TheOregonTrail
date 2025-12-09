SET(SOURCE_FILES
	base.c
	input.c
	main.c
	settings.c
	state.c
	static.c
	store.c
	tui.c
	utils.c
	blackjack/game.c
	blackjack/card.c
  screens/learn.c
  screens/main_settings.c
  screens/prepare.c
  screens/trail.c
  screens/map.c
  screens/general_store.c
)

if (WIN32)
	list(APPEND SOURCE_FILES ${VersionFilesOutputVariable})
endif()

if (NOT MUTE)
	list(APPEND SOURCE_FILES music.c)
endif()

if (DISCORD)
	list(APPEND SOURCE_FILES discord.c)
endif()

# this should be cleaned up later
file(WRITE "${CMAKE_BINARY_DIR}/licenses.c" "#include \"licenses.h\"\nconst SoftwareLicense LICENSES[] = {\n")
macro(add_license name path)
        file(READ "${CMAKE_SOURCE_DIR}/lib/${path}" filedata)
        # this sanitization is not perfect but good enough
        string(REPLACE \" \\\" filedata ${filedata})
        string(REPLACE \n \\n filedata ${filedata})
        file(APPEND "${CMAKE_BINARY_DIR}/licenses.c" "{\"${name}\", \"${filedata}\"},\n")
	list(APPEND SOURCE_FILES "${CMAKE_BINARY_DIR}/licenses.c")
endmacro()

add_license("Discord RPC" discord-rpc/LICENSE)
add_license("cfgpath.h" cfgpath/LICENSE)
add_license("Native File Dialog Extended" nativefiledialog-extended/LICENSE)
add_license("PortAudio" portaudio/LICENSE.txt)
add_license("c-vector" cvector/LICENSE)

file(APPEND "${CMAKE_BINARY_DIR}/licenses.c" "};\nconst unsigned LICENSES_COUNT = sizeof(LICENSES)/sizeof(*LICENSES);")

add_executable(tot ${SOURCE_FILES})
set_target_properties(tot PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

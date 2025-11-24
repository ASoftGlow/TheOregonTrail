SET(SOURCE_FILES
	base.c
	input.c
	main.c
	map.c
	settings.c
	state.c
	static.c
	store.c
	tui.c
	utils.c
	blackjack/game.c
	blackjack/card.c
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

# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "")
    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
		string(REGEX MATCH "[^.]+" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "static const unsigned char ${filename}[] = {${filedata}};\n")
    endforeach()
endfunction()

create_resources(${PROJECT_SOURCE_DIR}/resources/lang "${PROJECT_BINARY_DIR}/generated/langs.h")
list(APPEND SOURCE_FILES)

add_executable(tot ${SOURCE_FILES})
set_target_properties(tot PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
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

add_executable(tot ${SOURCE_FILES})
set_target_properties(tot PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

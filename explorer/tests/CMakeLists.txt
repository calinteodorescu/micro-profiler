cmake_minimum_required(VERSION 2.8)

set(MP_OUTDIR $<TARGET_FILE_DIR:explorer.tests>)

include(test)

set(EXPLORER_TEST_SOURCES
	ProcessExplorerTests.cpp
)

add_custom_command(OUTPUT copy-guineas.x
	COMMAND ${CMAKE_COMMAND} -E copy ${MP_OUTDIR}/guinea_runner.exe ${MP_OUTDIR}/guinea_runner2.exe
	COMMAND ${CMAKE_COMMAND} -E copy ${MP_OUTDIR}/guinea_runner.exe ${MP_OUTDIR}/guinea_runner3.exe
	COMMENT "Cloning guinea runners..."
)

add_library(explorer.tests SHARED ${EXPLORER_TEST_SOURCES} copy-guineas.x)
target_link_libraries(explorer.tests explorer ipc scheduler logger mt test-helpers)
add_dependencies(explorer.tests guinea_runner)

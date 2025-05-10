cmake_minimum_required (VERSION 3.10)

project (halley-launcher)

set(REQUIRE_WINDOWS_10 1)
set(HALLEY_PROJECT_EMBED 1)
set(HALLEY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/halley)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${HALLEY_PATH}/cmake/")
include(HalleyProject)

set(USE_SSL 1)

if(NOT (DEFINED HALLEY_GAME_BIN_DIR))
	set(HALLEY_GAME_BIN_DIR "bin")
endif()

if (MSVC)
	set (RESOURCES
		"res/halley-launcher.rc"
		"res/resource.h"
		"res/halley.ico"
	)
endif ()

file(STRINGS "source_list.txt" SOURCES)
set_directory_properties(PROPERTIES CMAKE_CONFIGURE_DEPENDS "source_list.txt")

halleyProjectV2(halley-launcher "${SOURCES}" "${RESOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/${HALLEY_GAME_BIN_DIR})

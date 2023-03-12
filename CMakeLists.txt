cmake_minimum_required (VERSION 3.10)

project (halley-launcher)

set(REQUIRE_WINDOWS_10 1)
set(HALLEY_PROJECT_EMBED 1)
set(HALLEY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/halley)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${HALLEY_PATH}/cmake/")
include(HalleyProject)

if(NOT (DEFINED HALLEY_GAME_BIN_DIR))
	set(HALLEY_GAME_BIN_DIR "bin")
endif()

set (SOURCES
	"prec.cpp"

	"src/choose_project.cpp"
	"src/launch_project.cpp"
	"src/launcher.cpp"
	"src/launcher_save_data.cpp"
	"src/launcher_stage.cpp"
	"src/project_properties.cpp"
	"src/settings.cpp"
	)

set (HEADERS
	"prec.h"

	"src/choose_project.h"
	"src/launch_project.h"
	"src/launcher.h"
	"src/launcher_save_data.h"
	"src/launcher_stage.h"
	"src/project_properties.h"
	"src/settings.h"
	)

if (MSVC)
	set (RESOURCES
		"res/halley-launcher.rc"
		"res/resource.h"
		"res/halley.ico"
	)
endif ()

#include_directories(${FREETYPE_INCLUDE_DIR} "../tools/include")

halleyProject(halley-launcher "${SOURCES}" "${HEADERS}" "${RESOURCES}" "${GEN_DEFINITIONS}" ${CMAKE_CURRENT_SOURCE_DIR}/${HALLEY_GAME_BIN_DIR})

#target_link_libraries (halley-launcher halley-tools)

cmake_minimum_required(VERSION 3.10)
project(tsab)

option(TSAB_EMBED_BYTECIDE "Embed main.lbc" OFF)
option(TSAB_BUILD_BINARY "Build the tsab executable" ON)
option(TSAB_TARGET_PREBUILD_SDL "Use prebuilt SDL" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# (-no-pie) is required on linux to be marked as executable in some cases (weird, I know)
set(CMAKE_CXX_FLAGS "-no-pie -Wno-return-type-c-linkage -Wno-format-security")

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_AR "em++")
	set(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
	set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

if (NOT TSAB_TARGET_PREBUILD_SDL)
	find_package(SDL2 REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(SDL2_image REQUIRED)
	find_package(SDL2_mixer REQUIRED)

	find_package(OpenGL REQUIRED)
	find_package(GLUT REQUIRED)

	set(${SDL2_INCLUDE_DIR} "${SDL2_INCLUDE_DIR}/SDL2/")
	include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS}/SDL2/ ${SDL2_IMAGE_INCLUDE_DIR}/SDL2/ ${SDL2_MIXER_INCLUDE_DIR}/SDL2/)
endif()

include_directories(include/ libs/lit/include/ libs/ libs/box2d/include/ libs/imgui/ libs/SDL_gpu/include/)

option(BOX2D_BUILD_DOCS "" OFF)
option(BOX2D_BUILD_UNIT_TESTS "" OFF)
option(BOX2D_BUILD_TESTBED "" OFF)

add_subdirectory(libs/box2d)
add_subdirectory(libs/lit)
add_subdirectory(libs/SDL_gpu)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/tsab/*.cpp)

if (EMSCRIPTEN)
	set(TSAB_EMBED_BYTECODE ON)
endif()

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dist/default_font.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile default_font ${CMAKE_CURRENT_SOURCE_DIR}/assets/default_font.ttf
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/default_font.ttf)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dist/gamecontrollerdb.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile gamecontrollerdb ${CMAKE_CURRENT_SOURCE_DIR}/assets/gamecontrollerdb.txt
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/gamecontrollerdb.txt)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dist/prefix.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile prefix ${CMAKE_CURRENT_SOURCE_DIR}/assets/prefix.lit
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/prefix.lit)

set(embed_resources)
list(APPEND embed_resources ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/dist/default_font.c ${CMAKE_CURRENT_SOURCE_DIR}/dist/gamecontrollerdb.c ${CMAKE_CURRENT_SOURCE_DIR}/dist/prefix.c
	${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/examples/imgui_impl_sdl.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/examples/imgui_impl_opengl3.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_draw.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui.cpp)

if(TSAB_EMBED_BYTECODE)
	MESSAGE("Embeding bytecode...")
	ADD_DEFINITIONS(-DEMBED_BYTECODE)

	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dist/bytecode.c
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile bytecode ${CMAKE_CURRENT_SOURCE_DIR}/main.lbc
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main.lbc)

	list(APPEND embed_resources ${CMAKE_CURRENT_SOURCE_DIR}/dist/bytecode.c)
endif()

add_library(tsab STATIC ${embed_resources})

if (TSAB_BUILD_ANDROID)
	set_target_properties(tsab PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist/${CMAKE_ANDROID_ARCH_ABI}")
	set_target_properties(tsab PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist/${CMAKE_ANDROID_ARCH_ABI}")
else()
	set_target_properties(tsab PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist")
	set_target_properties(tsab PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist")
endif()

if (TSAB_BUILD_BINARY)
	if (EMSCRIPTEN)
		add_definitions("-s BINARYEN_TRAP_MODE='clamp' -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 --shell-file src/html/shell.html -o index.html")
		set_target_properties(tsab PROPERTIES LINK_FLAGS "-s BINARYEN_TRAP_MODE='clamp' -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s USE_OGG=1 -s USE_VORBIS=1 --shell-file src/html/shell.html -o index.html")

		include_directories(${BOX2D_INCLUDE_DIR})
		add_executable(tsabc src/html/main.cpp)
	else()
		if (TSAB_TARGET_PREBUILD_SDL)
			target_link_libraries(tsab LINK_PUBLIC SDL2 SDL2_image SDL2_gfx SDL2_mixer SDL2_ttf)
		else()
			target_link_libraries(tsab LINK_PUBLIC ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARY} ${SDL2_MIXER_LIBRARY} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
		endif()

		add_executable(tsabc src/native/main.cpp)
	endif()

	target_link_libraries(tsabc LINK_PUBLIC tsab lit box2d SDL_gpu)
	set_target_properties(tsabc PROPERTIES OUTPUT_NAME "tsab")
	install(TARGETS tsabc DESTINATION bin)
elseif(TSAB_TARGET_PREBUILD_SDL)
	add_definitions(-DIMGUI_IMPL_OPENGL_ES2)
	target_link_libraries(tsab LINK_PUBLIC android EGL GLESv2 box2d lit SDL2 SDL2_image SDL2_gfx SDL2_mixer SDL2_ttf SDL_gpu)
endif()
cmake_minimum_required(VERSION 3.10)
project(tsab)

add_subdirectory(libs/lit)
option(EMBED_BYTECIDE "Embed main.lbc" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Required on linux to be marked as executable in some cases (weird, I know)
set(CMAKE_CXX_FLAGS "-no-pie -Wno-return-type-c-linkage -Wno-unused-command-line-argument")

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_AR "em++")
	set(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
	set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

include_directories(include/ libs/include/ libs/lit/include/ libs/ libs/box2d/include/)

if (EMSCRIPTEN)
	set(EMBED_BYTECODE ON)
	file(GLOB_RECURSE SOURCE_FILES src/html/*.cpp src/tsab/*.cpp)
else()
	file(GLOB_RECURSE SOURCE_FILES src/native/*.cpp src/tsab/*.cpp)
endif()

add_custom_command(
	OUTPUT dist/default_font.c
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile default_font ${CMAKE_CURRENT_SOURCE_DIR}/assets/default_font.ttf
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/default_font.ttf)

add_custom_command(
	OUTPUT dist/gamecontrollerdb.c
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile gamecontrollerdb ${CMAKE_CURRENT_SOURCE_DIR}/assets/gamecontrollerdb.txt
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/gamecontrollerdb.txt)

add_custom_command(
	OUTPUT dist/prefix.c
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/embedfile prefix ${CMAKE_CURRENT_SOURCE_DIR}/assets/prefix.lit
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/prefix.lit)

set(embed_resources)
list(APPEND embed_resources ${SOURCE_FILES} dist/default_font.c dist/gamecontrollerdb.c dist/prefix.c)

IF(EMBED_BYTECODE)
	MESSAGE("Embeding bytecode...")
	ADD_DEFINITIONS(-DEMBED_BYTECODE)

	add_custom_command(
		OUTPUT bytecode.c
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ./util/embedfile bytecode ${CMAKE_CURRENT_SOURCE_DIR}/main.lbc
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main.lbc)

	list(APPEND embed_resources dist/bytecode.c)
ENDIF()

add_executable(tsab ${embed_resources})

if (EMSCRIPTEN)
	add_definitions("-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 --shell-file src/html/shell.html -o index.html")
	set_target_properties(tsab PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s USE_OGG=1 -s USE_VORBIS=1 --shell-file src/html/shell.html -o index.html")

	include_directories(${BOX2D_INCLUDE_DIR})
	target_link_libraries(tsab ${CMAKE_SOURCE_DIR}/libs/lit/dist/liblit.a ${CMAKE_SOURCE_DIR}/libs/box2d/src/libbox2d.a ${CMAKE_SOURCE_DIR}/libs/lit/dist/liblit.bc ${CMAKE_SOURCE_DIR}/libs/SDL_GPU/libSDL2_gpu.a ${CMAKE_SOURCE_DIR}/libs/SDL_GPU/libSDL2_gpu.bc)
else()
	find_package(SDL2 REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(SDL2_image REQUIRED)
	find_package(SDL2_mixer REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(GLUT REQUIRED)

	include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR})
	target_link_libraries(tsab lit ${CMAKE_SOURCE_DIR}/libs/box2d/build/src/libbox2d.a ${CMAKE_SOURCE_DIR}/libs/SDL_GPU/libSDL2_gpu.a ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARY} ${SDL2_MIXER_LIBRARY})
endif()

install(TARGETS tsab DESTINATION bin)
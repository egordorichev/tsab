cmake_minimum_required(VERSION 3.10)
project(embedfile)

add_executable(embedfile embed_file.c)

set_target_properties(embedfile PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dist")
set_target_properties(embedfile PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dist")
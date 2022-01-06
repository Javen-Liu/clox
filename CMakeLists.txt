cmake_minimum_required(VERSION 3.17)
project(clox C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

aux_source_directory(. SRC_LIST)

link_libraries(m)

add_executable(clox
        chunk.c
        chunk.h
        common.h
        main.c
        memory.c
        memory.h
        debug.h
        debug.c value.c value.h vm.c vm.h rainbow.c rainbow.h compiler.c compiler.h scanner.c scanner.h object.c object.h table.c table.h)

add_library(lox SHARED ${SRC_LIST})

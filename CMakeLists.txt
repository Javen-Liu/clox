cmake_minimum_required(VERSION 3.17)
project(clox C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(clox
        chunk.c
        chunk.h
        common.h
        main.c
        memory.c
        memory.h
        debug.h
        debug.c value.c value.h vm.c vm.h)

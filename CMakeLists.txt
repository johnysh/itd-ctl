cmake_minimum_required(VERSION 3.13)
project(itd-ctl C)
set (PROGRAM_NAME itd-ctl)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_C_STANDARD 11)


add_executable(itd-ctl itd_ctl.c)
#target_include_directories(${PROGRAM_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/bits/)

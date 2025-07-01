#-------------------------------------------------------------------------------
# Copyright (c) 2019 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(Zif
        LANGUAGES CXX
        VERSION 0.6.2)

add_subdirectory(Platform)

#-------------------------------------------------------------------------------

add_executable(zif
               Source/zif.cpp
               Source/common/ConsoleImpl.cpp)

target_compile_definitions(zif PRIVATE TERMINAL_EMULATOR)

target_include_directories(zif PRIVATE Source)

target_link_libraries(zif PRIVATE GUI STB)

#-------------------------------------------------------------------------------

install(TARGETS zif RUNTIME DESTINATION .)
install(FILES README DESTINATION .)
install(DIRECTORY Images DESTINATION .)
install(DIRECTORY Games DESTINATION .)
install(DIRECTORY Scripts DESTINATION .)

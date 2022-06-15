#-------------------------------------------------------------------------------
#  Copyright (c) 2019 John D. Haughton
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.11.2)

project(Zif
        LANGUAGES C CXX ASM
        VERSION 0.6.2)

include(Platform/config.cmake)

#-------------------------------------------------------------------------------

add_executable(zif
               Source/zif.cpp
               Source/common/ConsoleImpl.cpp)

target_compile_definitions(zif PRIVATE TERMINAL_EMULATOR)

target_include_directories(zif PRIVATE Source)

target_link_libraries(zif PLT)

#-------------------------------------------------------------------------------

install(TARGETS zif RUNTIME DESTINATION .)
install(FILES README DESTINATION .)
install(DIRECTORY Images DESTINATION .)
install(DIRECTORY Games DESTINATION .)
install(DIRECTORY Scripts DESTINATION .)

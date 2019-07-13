
set(source  Source/zif.cpp Source/common/ConsoleImpl.cpp)
set(binary  zif)
set(app     Zif)
set(version 0.6.0-Beta)

add_compile_options(-DTERMINAL_EMULATOR)
include_directories(Source)

#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.6)
include(Platform/build.cmake NO_POLICY_SCOPE)

#-------------------------------------------------------------------------------

add_executable(${binary} ${source})
target_link_libraries(${binary} ${platform_libs})

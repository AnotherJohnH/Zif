
set(source  Source/zif.cpp Source/common/ConsoleImpl.cpp)
set(binary  zif)
set(app     Zif)
set(version 0.6.0-Beta)

add_compile_options(-DTERMINAL_EMULATOR)
include_directories(Source)

configure_file(README README COPYONLY)
file(COPY Games   DESTINATION .)
file(COPY Images  DESTINATION .)
file(COPY Scripts DESTINATION .)

set(pkg_source ${binary} README Games Images Scripts)

#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.11.4)
include(Platform/build.cmake NO_POLICY_SCOPE)

#-------------------------------------------------------------------------------

add_executable(${binary} ${source})
target_link_libraries(${binary} ${PLT_libs})

# Zif

A Z-code engine for playing interactive fiction games.

Portability of the application between different platforms is a particular goal, and specifically
portability to the third generation Kindle (aka Kindle Keyboard) which appears as an ideal
platform for playing interactive fiction.

The application includes a terminal emulator and a basic curses style front-end menu. So when
running on the Kindle a third-party terminal emulator and launcher is not necessary.

Perfectly good Z-code engines already exist and some have already been ported to the Kindle.
This project is just for the fun of writing code and the learning that brings.

## Status

[![Build Status](https://travis-ci.org/AnotherJohnH/Zif.svg?branch=master)](https://travis-ci.org/AnotherJohnH/Zif)

The application is currently beta quality. It has been built and seen to run on various platforms.
Almost all Z-code files, .z# and .zblorb, downloaded from the interactive fiction archive, start
to run as expected. (819/817) Although further testing is necessary, initial indications are that
there is enough functionality in place to play many of the available games.

|Target|Build|Run|
|---|---|---|
|Linux|OK|OK|
|macOS|OK|OK|
|Kindle3|OK|OK|
|Android|OK|Only using third party terminal e.g. --term|
|Emscripten|OK|Not working, only simple command line options --help etc.|
|iOS|Links (but no app yet)|-|

## How to run

Running with no command line arguments will load the default launcher configuration file
("zif.cfg") from the current working directory and will start the launcher front-end using the
built-in terminal emulator.

The command line option --help (or -h) provides basic help. Supplying a Z-code game
file as a command line argument will load and run the game file directly bypassing
the front-end.

The launcher configuration file ("zif.cfg") file is used to provide a front-end menu where
z-code games available in the local file system can be selected and certain aspects of
the terminal emulation configured.

## Thanks & Acknowledgements

Graham Nelson for his "Z-Machine Standards Document" and test programs. Andrew Plotkin
for his "Z-machine Exerciser". The contributers to Frotz, which has been an invaluable
reference of correct Z-Machine behaviour. The Z-Code authors, and everyone else
involved, for enabling their Z-code files to be freely available at the Interactive
Fiction Archive (http://ifarchive.org/) and the Interactive Fiction Database
(http://ifdb.tads.org/).

## How to build

Use scons in the top level directory to run the SConstruct file. e.g.

```
scons
```

The build files will work out whether the host system is Linux or macOS and configure the
build environment for the host system as the target. The automatic target selection can be
overriden by setting the PROJ\_TARGET environment variable. e.g.

```
   PROJ_TARGET=macOS
```

or

```
   PROJ_TARGET=Linux
```

Cross targets are also selected via the PROJ\_TARGET environment variable. e.g.

```
   PROJ_TARGET=Kindle3
```

The BUILD\_... .sh scripts are provided to initialise PROJ\_TARGET and set other
environment variables required by each specific build. These scripts should
be sourced and then the top level SConstruct invoked in the normal way.

### Linux and macOS

Depend on SDL2 and SDL2-image, so a development installs of SDL2 and SDL2-image are required.

### Kindle3

Although also a Linux build, the Kindle3 build does not depend on SDL2 or SDL2-image.

Requires gcc built for arm-linux-gnueabihf and a set of headers and static runtime libraries
that are compatible with the Linux installed on the Kindle3. The original ARMv6 Raspberry Pi
running a Debian based Linux has been found to be suitable platform to build the Kindle3 version.

### Browser (via Emscripten)

Uses the SDL2 library supplied with Emscripten. Some re-work/conditonal compilation will
be required to make it work. (TODO)

### iOS

Requires access to a source installation of SDL2 which is built into a linSDL2.a as part of
the application build process.

Build of an actual app has not been achieved yet, and may not be easily achievable without
an Xcode project file.

### Android

Requires an NDK installation. Currently builds as a console only app as an integration with
an Android build of SDL2 has not been achieved yet.

## Testing

Regression testing is mostly achieved via the [ZifTest](https://github.com/AnotherJohnH/ZifTest/)
project.

## Coding style

The source is C++ but has the following non-typical for modern C++ features ...
* Dynamic memory allocation is mostly avoided
* Use of C style stdio API
* In some places have re-invented the wheel avoiding functionality that is in standard libraries
* 3 space indent

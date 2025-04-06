# Setup liblzma on Windows for Lua (MinGW-w64 from MSYS2)

This page describes the process to setup ```liblzma``` on Windows, building it with the MinGW-w64 toolchain from MSYS2. 

## Overview

The easiest way to acquire ```liblzma``` on any MSYS2 environment for MinGW-w64 goes through the package manager for MSYS2 (```pacman```). Through ```pacman```, a complete ecossystem for Lua can be configured and installed quickly, including:

* C compiler
* Lua
* LuaRocks
* liblzma

## Setup a C compiler, Lua, LuaRocks and liblzma

1. First, choose a 64-bit MSYS2 environment for MinGW-w64 (```MINGW64```, ```UCRT64```, ```CLANG64```) for x86/x64 or (```CLANGARM64```) if you are running Windows on ARM. Once you decided your choice, launch the shell for that environment;

2. Install the packages from MSYS2 repositories for the chosen environment

    ```bash
    pacman -S ${MINGW_PACKAGE_PREFIX}-cc ${MINGW_PACKAGE_PREFIX}-lua ${MINGW_PACKAGE_PREFIX}-lua-luarocks ${MINGW_PACKAGE_PREFIX}-xz
    ```

3. You are ready to install ```lua-xz``` through ```luarocks```.

[Back to docs](./README.md#documentation)
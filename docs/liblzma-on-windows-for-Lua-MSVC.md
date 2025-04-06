# Setup liblzma on Windows for Lua (MSVC)

This page describes the process to setup ```liblzma``` on Windows, building it with Microsoft Visual Studio C/C++ (MSVC) toolchain. 

## Overview

The easiest way to build ```liblzma``` from the source code with the MSVC toolchain goes through [CMake](https://cmake.org/) build system.

## Table of Contents

* [Requirements](#requirements)
* [Building liblzma](#building-liblzma)

### Requirements

1. Download and install ```cmake```;
2. Open the ```Visual Studio Developer Command Prompt``` that you used to build Lua;
3. Assuming that [LuaRocks](https://luarocks.org/) is installed and configured on your system, and that it is also available on your system environment PATH variable, in the ```cmd``` prompt above, find the directory used by ```luarocks``` to hold external dependencies:

    ```cmd
    luarocks config "external_deps_dirs[1]"
    ```

> [!TIP]
> 
> The output of the command above should be something like ```c:\external``` if you didn't change it. If your home drive is ```C:``` as it is for most users, you are ok to proceed. However, if your home drive is something else like ```D:```, you can to change the external dependencies directory running the command:
> 
>    ```cmd
>    luarocks config "external_deps_dirs[1]" "D:\external"
>    ```

4. Once you found the directory for external dependencies, do not forget to add (```c:\external\bin``` or ```D:\external\bin``` if your home drive is D:) to the system environment PATH variable either temporarily or permanently (I prefer to have it added permanently on my setup). Moreover, for the next steps, I'll assume that you are running a recent enough Windows version to have ```tar``` and ```curl``` installed on your system by default (see [Windows 10 Insider build 17063 and later now include ```tar``` and ```curl```](https://devblogs.microsoft.com/commandline/tar-and-curl-come-to-windows/));

5. Confirm that all the required tools are installed on your system by displaying their versions:

    ```cmd
    cl
    lua -v
    luarocks --version
    cmake --version
    tar --version
    curl --version
    ```

### Building liblzma

Once you finished the initial steps in the [Requirements](#requirements) section, you are ready to build liblzma.

> [!NOTE]
> 
> In the following commands, each character is important, because it was tested and confirmed to work. So, do not erase any character if you are not confident on what you are doing.

1. Close any opened instances, and open again the ```Visual Studio Developer Command Prompt``` that you used to build Lua;

2. Change directory to the temporary directory, create a clean directory to build ```liblzma``` (potentially removing old directories) and change directory to it

    ```cmd
    cd "%TEMP%"
    IF EXIST ".\xz\" RMDIR /S /Q ".\xz\"
    mkdir ".\xz\"
    cd ".\xz\"
    ```

3. Set a cmd variable to hold the latest ```liblzma``` version (at the time of writing, it is ```5.8.1```). Then, download the source code with ```curl``` and extract it with ```tar```

    ```cmd
    SET LIBLZMA_VERSION=5.8.1
    curl -L "https://github.com/tukaani-project/xz/releases/download/v%LIBLZMA_VERSION%/xz-%LIBLZMA_VERSION%.tar.gz"
    tar -xf "xz-%LIBLZMA_VERSION%.tar.gz"
    ```

4. Set a variable (```INSTALL_DIR```) to hold the external directory pointed by ```luarocks```

    ```cmd
    FOR /F "usebackq tokens=*" %I IN (`luarocks config external_deps_dirs[1]`) DO ( SET "INSTALL_DIR=%I" )
    ```

5. Remove previous build directories (```liblzma-build-dir```) if they exist, configure the build of ```liblzma```, then build ```liblzma``` and install it

    ```cmd
    IF EXIST ".\liblzma-build-dir\" RMDIR /Q /S ".\liblzma-build-dir\"
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON --install-prefix "%INSTALL_DIR%" -S "xz-%LIBLZMA_VERSION%" -B liblzma-build-dir
    cmake --build liblzma-build-dir --config Release
    cmake --install liblzma-build-dir --config Release
    ```

6. If ```liblzma``` was built and installed properly, then change directory to the temporary directory again and delete the initial directory created to build the library

    ```cmd
    cd "%TEMP%"
    IF EXIST ".\xz\" RMDIR /Q /S ".\xz\"
    ```

7. You are ready to install ```lua-xz``` through ```luarocks```.

[Back to docs](./README.md#documentation)
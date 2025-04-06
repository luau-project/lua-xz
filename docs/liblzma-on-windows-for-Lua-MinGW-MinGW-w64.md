# Setup liblzma on Windows for Lua (Generic MinGW / MinGW-w64)

This page describes the process to setup ```liblzma``` on Windows, building it with a generic MinGW / MinGW-w64 toolchain.

## Overview

The easiest way to build ```liblzma``` from the source code with a generic MinGW / MinGW-w64 toolchain goes through [CMake](https://cmake.org/) build system, paired with [GNU Make](https://www.gnu.org/software/make/) for MinGW / MinGW-w64 (```mingw32-make```) or [Ninja](https://ninja-build.org/).

## Table of Contents

* [Requirements](#requirements)
* [Building liblzma](#building-liblzma)

### Requirements

1. Download and install ```cmake```;

2. If ```mingw32-make``` didn't come as part of your MinGW / MinGW-w64 installation, then download and install the latest ```ninja``` from [https://github.com/ninja-build/ninja/releases/latest](https://github.com/ninja-build/ninja/releases/latest), and place it on your system environment PATH variable;

3. Open a command prompt (```cmd```);

4. Assuming that [LuaRocks](https://luarocks.org/) is installed and configured on your system, and that it is also available on your system environment PATH variable, in the ```cmd``` prompt above, find the directory used by ```luarocks``` to hold external dependencies:

    ```cmd
    luarocks config "external_deps_dirs[1]"
    ```

> [!TIP]
> 
> The output of the command above should be something like ```c:\external``` or ```c:\mingw``` if you didn't change it. If your home drive is ```C:``` as it is for most users, you are ok to proceed. However, if your home drive is something else like ```D:```, you can to change the external dependencies directory running the command:
> 
>    ```cmd
>    luarocks config "external_deps_dirs[1]" "D:\external"
>    ```

5. Once you found the directory for external dependencies, do not forget to add (```c:\external\bin``` or ```c:\mingw\bin``` for a home drive C:, or ```D:\external\bin``` or ```D:\mingw\bin``` if your home drive is D:) to the system environment PATH variable either temporarily or permanently (I prefer to have it added permanently on my setup). Moreover, for the next steps, I'll assume that you are running a recent enough Windows version to have ```tar``` and ```curl``` installed on your system by default (see [Windows 10 Insider build 17063 and later now include ```tar``` and ```curl```](https://devblogs.microsoft.com/commandline/tar-and-curl-come-to-windows/));

6. Confirm that all the required tools are installed on your system by displaying their versions:

    ```cmd
    gcc --version
    lua -v
    luarocks --version
    cmake --version
    tar --version
    curl --version
    ```

7. Confirm that either ```mingw32-make``` or ```ninja``` are installed by printing the version of the tool:

    * ```mingw32-make --version```
    * ```ninja --version```

### Building liblzma

Once you finished the initial steps in the [Requirements](#requirements) section, you are ready to build liblzma.

> [!NOTE]
> 
> In the following commands, each character is important, because it was tested and confirmed to work. So, do not erase any character if you are not confident on what you are doing.

1. Close any opened instances, and open again the command prompt (```cmd```);

2. Change directory to any folder without spaces (`%SystemDrive%\` will expand to `C:\` if your system was installed at C:), create a clean directory to build ```liblzma``` (potentially removing old directories) and change directory to it

    ```cmd
    cd "%SystemDrive%\"
    IF EXIST ".\xz\" RMDIR /S /Q ".\xz\"
    mkdir ".\xz\"
    cd ".\xz\"
    ```

> [!WARNING]
> 
> Old versions of GNU Make (```mingw32-make```) might fail to build on folders containing spaces in the path. So, play safe.

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

    * If you have ```mingw32-make```, then run

        ```cmd
        IF EXIST ".\liblzma-build-dir\" RMDIR /Q /S ".\liblzma-build-dir\"
        cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON --install-prefix "%INSTALL_DIR%" -S "xz-%LIBLZMA_VERSION%" -B liblzma-build-dir
        cmake --build liblzma-build-dir --config Release
        cmake --install liblzma-build-dir --config Release
        ```

    * Otherwise, for ```ninja```

        ```cmd
        IF EXIST ".\liblzma-build-dir\" RMDIR /Q /S ".\liblzma-build-dir\"
        cmake -G "Ninja" -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON --install-prefix "%INSTALL_DIR%" -S "xz-%LIBLZMA_VERSION%" -B liblzma-build-dir
        cmake --build liblzma-build-dir --config Release
        cmake --install liblzma-build-dir --config Release
        ```

6. If ```liblzma``` was built and installed properly, then change directory to the drive folder again and delete the initial directory created to build the library

    ```cmd
    cd "%SystemDrive%\"
    IF EXIST ".\xz\" RMDIR /Q /S ".\xz\"
    ```

7. You are ready to install ```lua-xz``` through ```luarocks```.

[Back to docs](../README.md#documentation)
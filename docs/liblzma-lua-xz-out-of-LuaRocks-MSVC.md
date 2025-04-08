# Setup liblzma and lua-xz out of LuaRocks (MSVC)

In the first part, this guide explains the process to setup ```liblzma``` on Windows. Then, it proceeds to build `lua-xz` out of LuaRocks.

## Table of Contents

* [Prerequisites](#prerequisites)
* [Build liblzma](#build-liblzma)
* [Build lua-xz](#build-lua-xz)

## Prerequisites
- **CMake**: Install the latest version of CMake from [https://cmake.org/download/](https://cmake.org/download/);
- **MSVC** + **Lua built with MSVC**: This guide assumes that you built Lua with MSVC;
- **Git**: Install the latest version of `git` for Windows from [https://git-scm.com/downloads](https://git-scm.com/downloads).

> [!NOTE]
> 
> For the next steps, I'll assume that you are running a recent enough Windows version to have ```tar``` and ```curl``` installed on your system by default (see [Windows 10 Insider build 17063 and later now include ```tar``` and ```curl```](https://devblogs.microsoft.com/commandline/tar-and-curl-come-to-windows/)). You can confirm that all the required tools are installed on your system by displaying their versions:
> 
>   ```cmd
>   cl
>   git --version
>   cmake --version
>   tar --version
>   curl --version
>   ```

## Build liblzma

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

4. Set a variable (```LIBLZMA_DIR```) to point to the directory where you want to install `liblzma`:

    ```cmd
    SET LIBLZMA_DIR=%SYSTEMDRIVE%\liblzma
    ```

> [!TIP]
> 
> The environment variable `%SYSTEMDRIVE%` points to the drive holding your Windows installation (e.g.:`C:`)

5. Remove previous build directories (```liblzma-build-dir```) if they exist, configure the build of ```liblzma```, then build ```liblzma``` and install it

    ```cmd
    IF EXIST ".\liblzma-build-dir\" RMDIR /Q /S ".\liblzma-build-dir\"
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON --install-prefix "%LIBLZMA_DIR%" -S "xz-%LIBLZMA_VERSION%" -B liblzma-build-dir
    cmake --build liblzma-build-dir --config Release
    cmake --install liblzma-build-dir --config Release
    ```

6. If ```liblzma``` was built and installed properly, then change directory to the temporary directory again and delete the initial directory created to build the library

    ```cmd
    cd "%TEMP%"
    IF EXIST ".\xz\" RMDIR /Q /S ".\xz\"
    ```

7. Keep the MSVC developer command prompt opened for the next steps.

## Build lua-xz

In order to build `lua-xz`, you need:

* `liblzma`, which you have installed at `%LIBLZMA_DIR%`
* `Lua` built with MSVC, which we are going to set a variable `%LUA_DIR%`

    ```cmd
    SET LUA_DIR=C:\path\to\Lua
    ```

    to hold the path to the directory of Lua.

* `lua-xz` source code, which you can clone from the `main` branch of this repository:

    ```cmd
    git clone https://github.com/luau-project/lua-xz
    ```

> [!TIP]
> 
> If your Lua interpreter (`lua.exe`) can be found at `C:\Lua\lua.exe`, then you shall set `%LUA_DIR%` as follows
> 
>   ```cmd
>   SET LUA_DIR=C:\Lua
>   ```

1. Change directory to the cloned `lua-xz` directory:

    ```cmd
    cd lua-xz
    ```

2. Now, assuming Lua 5.4 is installed at `%LUA_DIR%` and `liblzma` installed at `%LIBLZMA_DIR%`, we can change directory to `lua-xz`, then build it using `nmake` by running the command below:

    ```cmd
    nmake /F Makefile.msvc "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LIBLZMA_DIR=%LIBLZMA_DIR%"
    ```

> [!IMPORTANT]
> 
> The command above might fail if your Lua import library is not named in the same way as expected by the Makefile.msvc. To fix this, you can check the name of Lua import library at `%LUA_DIR%\lib`. At the moment, the Makefile assumes it is named `lua54.lib` for Lua 5.4 (thus, the ```%LUA_LIB%=lua54.lib``` is automatically set). You can override it, let's say to `lua.lib`, by running:
> 
>   ```cmd
>   nmake /F Makefile.msvc "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LUA_LIB=lua.lib" "LIBLZMA_DIR=%LIBLZMA_DIR%"
>   ```

3. If you managed to get a successful build, then you are ready to install `lua-xz` just by adding `install` at the end of the command that you used to have a successful build above:

    ```cmd
    nmake /F Makefile.msvc "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LIBLZMA_DIR=%LIBLZMA_DIR%" install
    ```

[Back to docs](./README.md#documentation), [Back to ToC](#table-of-contents)
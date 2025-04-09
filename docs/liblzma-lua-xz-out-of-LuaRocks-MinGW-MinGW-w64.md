# Setup liblzma and lua-xz out of LuaRocks (MinGW / MinGW-w64)

In the first part, this guide explains the process to setup ```liblzma``` on Windows. Then, it proceeds to build `lua-xz` out of LuaRocks.

## Table of Contents

* [Prerequisites](#prerequisites)
* [Build liblzma](#build-liblzma)
* [Build lua-xz](#build-lua-xz)
* [Possible issues building or installing lua-xz](#possible-issues-building-or-installing-lua-xz)

## Prerequisites
- **CMake**: Install the latest version of CMake from [https://cmake.org/download/](https://cmake.org/download/);
- **MinGW / MinGW-w64** + **Lua built with MinGW / MinGW-w64**: This guide assumes that you built Lua with MinGW / MinGW-w64;
- **GNU Make**: often found as `mingw32-make` on most distributions for Windows;
- **Git**: Install the latest version of `git` for Windows from [https://git-scm.com/downloads](https://git-scm.com/downloads).

> [!NOTE]
> 
> For the next steps, I'll assume that you are running a recent enough Windows version to have ```tar``` and ```curl``` installed on your system by default (see [Windows 10 Insider build 17063 and later now include ```tar``` and ```curl```](https://devblogs.microsoft.com/commandline/tar-and-curl-come-to-windows/)). You can confirm that all the required tools are installed on your system by displaying their versions:
> 
>   ```cmd
>   gcc --version
>   git --version
>   cmake --version
>   mingw32-make --version
>   tar --version
>   curl --version
>   ```

## Build liblzma

Once you finished the initial steps in the [Prerequisites](#prerequisites) section, you are ready to build liblzma.

> [!NOTE]
> 
> In the following commands, each character is important, because it was tested and confirmed to work. So, do not erase any character if you are not confident on what you are doing.

1. Close any opened instances, and open again the command prompt (```cmd```).  **Note**: if your Lua installation lies in a system-wide directory (`C:\Program Files\Lua`), then you must open the command prompt as administrator by launching it as `Run as administrator` to avoid permission issues on install;

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
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON --install-prefix "%INSTALL_DIR%" -S "xz-%LIBLZMA_VERSION%" -B liblzma-build-dir
    cmake --build liblzma-build-dir --config Release
    cmake --install liblzma-build-dir --config Release
    ```

6. If ```liblzma``` was built and installed properly, then change directory to the drive folder again and delete the initial directory created to build the library

    ```cmd
    cd "%SystemDrive%\"
    IF EXIST ".\xz\" RMDIR /Q /S ".\xz\"
    ```

7. Keep command prompt (`cmd`) opened for the next steps.

## Build lua-xz

In order to build `lua-xz`, you need:

* `liblzma`, which you have installed at `%LIBLZMA_DIR%`
* `Lua` built with MinGW / MinGW-w64, which we are going to set a variable `%LUA_DIR%`

    ```cmd
    SET LUA_DIR=C:\path\to\Lua
    ```

    to hold the path to the directory of Lua.

> [!TIP]
> 
> If your Lua interpreter (`lua.exe`) can be found at `C:\Lua\bin\lua.exe`, then you shall set `%LUA_DIR%` as follows
> 
>   ```cmd
>   SET LUA_DIR=C:\Lua
>   ```

* `lua-xz` source code, which you can clone from the `main` branch of this repository:

    ```cmd
    git clone https://github.com/luau-project/lua-xz
    ```

### Steps

1. Change directory to the cloned `lua-xz` directory:

    ```cmd
    cd lua-xz
    ```

2. Now, assuming Lua 5.4 is installed at `%LUA_DIR%` and `liblzma` installed at `%LIBLZMA_DIR%`, we can build `lua-xz` using `mingw32-make` by running the command below:

    ```cmd
    mingw32-make -f Makefile.mingw "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LIBLZMA_DIR=%LIBLZMA_DIR%"
    ```

> [!IMPORTANT]
> 
> If you face any issue during the execution of this step, check the section [Build issues](#build-issues) below.

3. If you managed to get a successful build, then you are ready to install `lua-xz` just by adding `install` at the end of the command that you used to have a successful build above:

    ```cmd
    mingw32-make -f Makefile.mingw "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LUA_LIB=lua" "LIBLZMA_DIR=%LIBLZMA_DIR%" install
    ```

> [!IMPORTANT]
> 
> If you face any issue during the execution of this step, check the section [Installation issues](#installation-issues) below.

## Possible issues building or installing lua-xz

Below, we list all the common issues that might apply when building `lua-xz` from the makefiles.

### Build issues

This section describes issues you might face when running the command above

```cmd
mingw32-make -f Makefile.mingw "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LIBLZMA_DIR=%LIBLZMA_DIR%"
```

#### Issues

1. Check your Lua version: in case it is Lua 5.1, then replace `"LUA_MINOR_VERSION=4"` by `"LUA_MINOR_VERSION=1"`

2. Check your Lua directory: if your interpreter is located at `C:\Program Files\Lua\bin\lua.exe`, then reset `%LUA_DIR%` variable by running

    ```cmd
    set "LUA_DIR=C:\Program Files\Lua"
    ```

and rerun the build command again.

3. Check the include directory of Lua: if the header files for Lua (`lua.h`, `luaconf.h`, `lualib.h`, `lauxlib.h`, `lua.hpp`) are located at `C:\Program Files\Lua\include\lua5.4`, then add the argument `"LUA_INCDIR=C:\Program Files\Lua\include\lua5.4"` to the build command as follows:

    ```cmd
    mingw32-make -f Makefile.mingw "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LUA_INCDIR=C:\Program Files\Lua\include\lua5.4" "LIBLZMA_DIR=%LIBLZMA_DIR%"
    ```

4. Check the import library of Lua: if the import library for Lua (a file with extension `.dll.a`, most likely named `lua.dll.a`, `lua-5.4.dll.a`, `lua54.dll.a`). If it happens to be `lua.dll.a`, then add the argument `"LUA_LIB=lua"` to the build command as follows:

    ```cmd
    mingw32-make -f Makefile.mingw "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LUA_LIB=lua" "LIBLZMA_DIR=%LIBLZMA_DIR%"
    ```

### Installation issues

This section describes issues you might face when running the command above

```cmd
mingw32-make -f Makefile.mingw "LUA_MAJOR_VERSION=5" "LUA_MINOR_VERSION=4" "LUA_DIR=%LUA_DIR%" "LIBLZMA_DIR=%LIBLZMA_DIR%" install
```

Here, all the issues covered on [Build issues](#build-issues) might happen, so double-check it.

> [!IMPORTANT]
> 
> Additionally, if your Lua installation resides in a side-wide directory with protected permissions, you must run all the commands in an elevated command prompt (`Run as administrator`). If you got issues regarding describing permissions rights, one possible solution that might apply is running all the [build steps](#build-lua-xz) above as admin.

[Back to docs](./README.md#documentation), [Back to ToC](#table-of-contents)
## Documentation

Welcome to ```lua-xz``` documentation!

## Overview

On Unix, the process to have `Lua`, `LuaRocks` and `liblzma` installed is quite straightforward, because most distributions make these tools and libraries available through a package manager to their users. On the other hand, on Windows, the concept of software distribution from a centralized repository provided by Microsoft is not a reality. 

Then, the main goal here is to guide Lua users to build `liblzma` and `lua-xz` on Windows. In the long run, the easiest way to manage Lua modules goes through the use of LuaRocks. However, due the fact that many Lua users on Windows struggle to setup LuaRocks correctly, we also describe how to build `liblzma` and `lua-xz` out of LuaRocks.

> [!NOTE]
> 
> To make things harder on Windows, in general, the building process or installation method varies according to the C toolchain used to build Lua.

If you have `LuaRocks` installed and configured correctly on your Windows system, then read the appropriate guide for your C toolchain in the section [To play nicely with LuaRocks](#to-play-nicely-with-luarocks). Otherwise, in case you find it too difficult to setup LuaRocks, go to [Out of LuaRocks](#out-of-luarocks).

## Setup liblzma and lua-xz on Windows

In short, we provide two manners to build and install `liblzma` + `lua-xz`.

1. [To play nicely with LuaRocks](#to-play-nicely-with-luarocks)
2. [Out of LuaRocks](#out-of-luarocks)

### To play nicely with LuaRocks

* Lua built with MSVC toolchain: follow the [MSVC guide](./liblzma-on-windows-for-Lua-MSVC.md);
* Lua built with MinGW-64 from [MSYS2](https://www.msys2.org/): follow the [MSYS2 guide](./liblzma-on-windows-for-Lua-MSYS2.md);
* Lua built with ```gcc``` from [chocolatey](https://chocolatey.org/), [scoop](https://scoop.sh/), [winlibs](https://winlibs.com/), [sourceforge](https://sourceforge.net/projects/mingw/) or [osdn.net](https://osdn.net/projects/mingw/): follow the [generic MinGW / MinGW-64 guide](./liblzma-on-windows-for-Lua-MinGW-MinGW-w64.md).

### Out of LuaRocks

* Build `liblzma` and `lua-xz` (MSVC): read the [MSVC guide](./liblzma-lua-xz-out-of-LuaRocks-MSVC.md)
* Build `liblzma` and `lua-xz` (MinGW / MinGW-w64): read the [MinGW / MinGW-w64 guide](./liblzma-lua-xz-out-of-LuaRocks-MinGW-MinGW-w64.md)

[Back to home](../)
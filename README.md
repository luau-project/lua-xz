# lua-xz

[![CI](https://github.com/luau-project/lua-xz/actions/workflows/ci.yml/badge.svg)](./.github/workflows/ci.yml)

> [!IMPORTANT]
> 
> **DO NOT** use this library while it is not available on [LuaRocks](https://luarocks.org/), because it is not ready yet. It is under development / testing before the initial release.

## Overview

**lua-xz** is a lightweight, native library for Lua providing a streaming interface to read/write xz files. To do so, ```lua-xz``` uses the general-purpose data compression [liblzma](https://github.com/tukaani-project/xz) library.

> [!NOTE]
> 
> ```lua-xz``` is implemented in C, and also compiles as C++.

## Table of Contents

* [Installation](#installation)
* [Usage](#usage)
* [Constants](#constants)
* [Classes](#classes)
    * [stream (reader)](#stream-reader)
    * [stream (writer)](#stream-writer)
* [Known limitations](#known-limitations)
* [Change log](#change-log)
* [Future works](#future-works)

## Installation

> [!IMPORTANT]
> 
> On each operating system or software distribution, ```lua-xz``` depends on the ```liblzma``` library:
>   * On Windows, see the [docs](./docs/README.md#documentation) to install ```liblzma```;
>   * On Debian-based (e.g.: Ubuntu) distributions:
> 
>     ```bash
>     sudo apt install -y liblzma-dev
>     ```
> 
>   * On RedHat-based (e.g.: Fedora) distributions:
> 
>     ```bash
>     sudo dnf install -y xz-devel
>     ```
> 
>   * On FreeBSD, it is already included in the base system.

Assuming that both ```liblzma``` and [LuaRocks](https://luarocks.org) are properly installed and configured on your system, execute the following command:

```bash
luarocks install lua-xz
```

## Usage

### Compress a file

```lua
-- load the library
local xz = require("lua-xz")

-- the file to compress
local filename = "README.md"

-- compressed file name
local compressed_filename = filename .. ".xz"

-- create a writer stream
-- first parameter:
--  the compression preset
-- second parameter:
--  the integrity check
-- note:
--  1) preset can be:
--      * an integer [0, 9]
--      * a string "0", ..., "9"
--      * a string "0e", ..., "9e" (preset level + extreme modifier)
--  2) if the .xz file needs to be
--  decompressed with XZ Embedded, use
--  xz.CHECK_CRC32 instead.
local stream = xz.stream.writer(xz.PRESET_DEFAULT, xz.CHECK_CRC64)

-- open the input file to feed
-- its content to the compression stream
local input = assert(
    io.open(filename, "rb"),
    "failed to open " .. filename .. " file for reading"
)

-- open / create the output file to hold
-- the compressed data
local output = assert(
    io.open(compressed_filename, "wb"),
    "failed to open  " .. compressed_filename .. " file for writing"
)

-- define the number of bytes
-- of the chunk to be read
-- from the input file
local chunk_size = 64

-- read the chunk from file
local chunk = input:read(chunk_size)

-- keep a variable to receive
-- the compressed chunk from the
-- writer stream
local compressed_chunk

-- keep reading the file
-- while there is data to read
while (chunk ~= nil) do
    -- feed the stream with the chunk
    -- and get the compressed chunk
    compressed_chunk = stream:update(chunk)

    -- write the compressed chunk
    -- to the output file
    output:write(compressed_chunk)

    -- read the next chunk from the input file
    chunk = input:read(chunk_size)
end

-- finish the stream and get
-- the last compressed chunk
compressed_chunk = stream:finish()

-- write the compressed chunk
-- to the output file
output:write(compressed_chunk)

-- close the output file
output:close()

-- close the input file
input:close()

-- close the writer stream to free resources
-- tip: it is automatically freed on garbage collection
stream:close()
```

### Decompress a file

```lua
-- load the library
local xz = require("lua-xz")

-- the file to decompress
local filename = "README.md.xz"

-- the decompressed file name
local decompressed_filename = "copy-of-" .. (filename:gsub("%.xz$", ""))

-- create a reader stream
-- first parameter:
--  a memory limit in bytes
--  can be chosen
-- second parameter:
--  decoding flags
-- note: 1) xz.MEMLIMIT_UNLIMITED does not use a limit
--       2) xz.CONCATENATED decodes all the streams,
--       not only the first stream.
local stream = xz.stream.reader(xz.MEMLIMIT_UNLIMITED, xz.CONCATENATED)

-- open the input file to feed
-- its content to the decompression stream
local input = assert(
    io.open(filename, "rb"),
    "failed to open " .. filename .. " file for reading"
)

-- open / create the output file to hold
-- the decompressed data
local output = assert(
    io.open(decompressed_filename, "wb"),
    "failed to open " .. decompressed_filename .. " file for writing"
)

-- define the number of bytes
-- of the chunk to be read
-- from the input file
local chunk_size = 64

-- read the chunk from file
local chunk = input:read(chunk_size)

-- keep a variable to receive
-- the decompressed chunk from the
-- reader stream
local decompressed_chunk

-- keep reading the file
-- while there is data to read
while (chunk ~= nil) do
    -- feed the stream with the chunk
    -- and get the decompressed chunk
    decompressed_chunk = stream:update(chunk)

    -- write the decompressed chunk
    -- to the output file
    output:write(decompressed_chunk)

    -- read the next chunk from the input file
    chunk = input:read(chunk_size)
end

-- finish the stream and get
-- the last decompressed chunk
decompressed_chunk = stream:finish()

-- write the decompressed chunk
-- to the output file
output:write(decompressed_chunk)

-- close the output file
output:close()

-- close the input file
input:close()

-- close the reader stream to free resources
-- tip: it is automatically freed on garbage collection
stream:close()
```

## Constants

| Key | Type | Description |
|---|---|---|
| version | string | Version of the binding (e.g.: 0.0.1) |
| _VERSION | string | Version of the underlying ```liblzma``` library (e.g.: 5.8.0) |
| MEMLIMIT_UNLIMITED | integer | A custom value in Lua to disable a memory limit |
| CONCATENATED | integer | Flag to enable decoding of concatenated files |
| PRESET_DEFAULT | integer | Default compression preset |
| CHECK_NONE | integer | No integrity check is calculated |
| CHECK_CRC32 | integer | Calculate CRC32 integrity check using the polynomial from the IEEE 802.3 standard |
| CHECK_CRC64 | integer | Calculate CRC64 integrity check using the polynomial from the ECMA-182 standard |
| CHECK_SHA256 | integer | SHA256 integrity check |

> [!NOTE]
> 
> Each constant above can be accessed through its key in the library. For instance, the underlying ```liblzma``` library version can be printed through the following code
> 
> ```lua
> local xz = require("lua-xz")
> print(xz._VERSION)
> ```

## Classes

In order to provide a streaming interface to read/write xz files, a core class ```stream``` is exposed such that its behavior comes in two flavours depending on the creation method: a ```reader``` stream and also a ```writer``` stream.

The stream class can be accessed through the ```stream``` key of the ```lua-xz``` library:

```lua
local xz = require("lua-xz")
-- call the method `xz.stream.reader' to create a reader stream
-- or the method `xz.stream.writer' to create a writer stream.
```

### stream (reader)

A stream to decompress data from .xz formatted content

#### Static methods

##### reader

* *Description*: Creates a reader stream to decompress data from .xz formatted content
* *Signature*: ```xz.stream.reader(memlimit, flags [, buffersize ])```
* *Parameters*: 
    * *memlimit* (```integer```): Memory usage limit as bytes. Use ```xz.MEMLIMIT_UNLIMITED``` to effectively disable the limiter;
    * *flags* (```integer```): Bitwise-or of zero or more of the decoder flags (for now, only ```xz.CONCATENATED``` is provided as constant);
    * *buffersize* (```integer | nil```): The size of the output buffer to allocate memory dynamically at stream creation. If no value is provided, it uses the default ```BUFSIZ``` value from the ```stdio.h``` header file;
* *Return* (```userdata```): An instance of the stream reader class.

#### Instance methods

##### close

* *Description*: Closes the reader stream and free resources
* *Signature*: ```stream:close()```
* *Parameters*: 
    * *stream* (```userdata```): An instance of the stream class;
* *Return* (```void```): Nothing.

##### finish

* *Description*: Finishes the decompression stream
* *Signature*: ```stream:finish()```
* *Parameters*: 
    * *stream* (```userdata```): An instance of the stream class;
* *Return* (```string```): The last decompressed chunk from content previously fed.

##### update

* *Description*: Feeds data to be decompressed
* *Signature*: ```stream:update(data)```
* *Parameters*: 
    * *stream* (```userdata```): An instance of the stream class;
    * *data* (```string```): The data to feed the stream;
* *Return* (```string```): The decompressed chunk from content previously fed.

### stream (writer)

A stream to compress data to .xz format

#### Static methods

##### writer

* *Description*: Creates a writer stream to compress data to .xz format
* *Signature*: ```xz.stream.writer(preset, check [, buffersize ])```
* *Parameters*: 
    * *preset* (```integer | string```): Compression level as an integer [0, 9] or string with a single digit [0-9] occasionally followed by 'e' character to indicate extreme compression preset. Valid values: 0, 1, 2, 6, 7, 9, "3", "5", "7", "5e", "6e", "8e" and so on;
    * *check* (```integer```): Type of the integrity check to calculate from uncompressed data;
    * *buffersize* (```integer | nil```): The size of the output buffer to allocate memory dynamically at stream creation. If no value is provided, it uses the default ```BUFSIZ``` value from the ```stdio.h``` header file.
* *Return* (```userdata```): An instance of the stream writer class.

#### Instance methods

##### close

* *Description*: Closes the writer stream and free resources
* *Signature*: ```stream:close()```
* *Parameters*: 
    * *stream* (```userdata```): An instance of the stream class;
* *Return* (```void```): Nothing.

##### finish

* *Description*: Finishes the compression stream
* *Signature*: ```stream:finish()```
* *Parameters*: 
    * *stream* (```userdata```): An instance of the stream class;
* *Return* (```string```): The last compressed chunk from content previously fed.

##### update

* *Description*: Feeds data to be compressed
* *Signature*: ```stream:update(data)```
* *Parameters*: 
    * *stream* (```userdata```): An instance of the stream class;
    * *data* (```string```): The data to feed the stream;
* *Return* (```string```): The compressed chunk from content previously fed.

## Known limitations

1. Due the fact that ```lua-xz``` depends on the ```liblzma``` library, which internally uses 64-bit integers on most of its APIs, ```lua-xz``` requires a Lua build that uses 64-bit integers. This means that the Lua type ```lua_Integer``` must be able to store at least 64 bits;
2. Again due the dependency on ```liblzma```, which requires a C compiler compliant with C99, ```lua-xz``` also requires C99 to build.

## Change log

* v0.0.1: Initial release

## Future works

* Add CMake as build system
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
    * [Compress a file](#compress-a-file)
    * [Decompress a file](#decompress-a-file)
    * [Simulate compression in chunks](#simulate-compression-in-chunks)
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
>   * On Windows, read the [docs](./docs/README.md#documentation) to learn how to install ```liblzma``` in a form that it plays nicely with Lua and LuaRocks;
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

Through the streaming interface, three examples are shown:

* Compress a file
* Decompress a file
* Simulate compression in chunks

### Compress a file

For this example, we compress the ```README.md``` file of this project, which might even become larger than the original due its small size. In the end of the script execution, a file named  ```README.md.xz``` is written to disk.

```lua
-- load the library
local xz = require("lua-xz")

-- the file to compress
local filename = "README.md"

-- compressed file name
local compressed_filename = filename .. ".xz"

-- create a writer stream
-- 
-- first parameter:
--  preset:
--    the compression preset
-- second parameter:
--  check:
--    the integrity check
-- 
-- note:
--  1) preset can be:
--      * an integer [0, 9]
--      * a string "0", ..., "9"
--      * a string "0e", ..., "9e" (preset level + extreme modifier)
--  2) if the .xz file needs to be
--  decompressed with XZ Embedded, use
--  xz.check.CRC32 instead.
-- 
-- tip: always check for errors
local ok, stream = pcall(
    function()
        return xz.stream.writer(xz.PRESET_DEFAULT, xz.check.CRC64)
    end
)

-- an error occurred ?
if (not ok) then
    -- raise the error
    error(stream)
end

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

-- define a producer function
-- to feed uncompressed data
-- to be encoded by the writer stream
local function producer()

    -- define the number of bytes
    -- to be read from the input file
    -- in a single chunk.
    -- In a real world scenario,
    -- 8kb (8 * 1024) would be
    -- a reasonable value
    local chunk_size = 64

    -- read the chunk from file
    local chunk = input:read(chunk_size)

    -- return the chunk read
    return chunk
end

-- define a consumer function
-- to handle compressed chunks
-- emitted by the writer stream
local function consumer(compressed_chunk)
    output:write(compressed_chunk)
end

do
    -- execute the stream
    -- 
    -- tip: always check for errors
    local ok, exec_err = pcall(
        function()
            stream:exec(producer, consumer)
        end
    )

    -- an error occurred ?
    if (not ok) then
        -- raise the error
        error(exec_err)
    end
end

-- close the writer stream to free resources
-- 
-- tip: it is automatically freed on garbage collection
stream:close()

-- close the output file
output:close()

-- close the input file
input:close()
```

### Decompress a file

For this example, we decompress the ```README.md.xz``` file created by [Compress a file](#compress-a-file) above, and we output a file named ```copy-of-README.md```, which should be a perfect copy of this ```README.md``` file.

```lua
-- load the library
local xz = require("lua-xz")

-- the file to decompress
local filename = "README.md.xz"

-- the decompressed file name
local decompressed_filename = "copy-of-" .. (filename:gsub("%.xz$", ""))

-- create a reader stream
-- 
-- first parameter:
--  a memory limit in bytes
--  can be chosen
-- second parameter:
--  decoding flags
-- 
-- note: 1) xz.MEMLIMIT_UNLIMITED does not use a limit
--       2) xz.CONCATENATED decodes all the streams,
--       not only the first stream.
-- 
-- tip: always check for errors
local ok, stream = pcall(
    function()
        return xz.stream.reader(xz.MEMLIMIT_UNLIMITED, xz.CONCATENATED)
    end
)

-- an error occurred ?
if (not ok) then
    -- raise the error
    error(stream)
end

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

-- define a producer function
-- to feed compressed data
-- to be decoded by the reader stream
local function producer()

    -- define the number of bytes
    -- to be read from the input file
    -- in a single chunk.
    -- In a real world scenario,
    -- 8kb (8 * 1024) would be
    -- a reasonable value
    local chunk_size = 64

    -- read the chunk from file
    local chunk = input:read(chunk_size)

    -- return the chunk read
    return chunk
end

-- define a consumer function
-- to handle decompressed chunks
-- emitted by the reader stream
local function consumer(decompressed_chunk)
    output:write(decompressed_chunk)
end

do
    -- execute the stream
    -- 
    -- tip: always check for errors
    local ok, exec_err = pcall(
        function()
            stream:exec(producer, consumer)
        end
    )

    -- an error occurred ?
    if (not ok) then
        -- raise the error
        error(exec_err)
    end
end

-- close the reader stream to free resources
-- 
-- tip: it is automatically freed on garbage collection
stream:close()

-- close the output file
output:close()

-- close the input file
input:close()
```

### Simulate compression in chunks

As a last example, we simulate compression in chunks. The idea is that data can come in chunks from HTTPS requests, and we feed a writer stream to compress the data coming from the internet to disk. In the end, decompression is performed on the saved file on disk, and the inputs are matched against the decompressed output.

```lua
local xz = require("lua-xz")

-- simulate content to be streamed to the encoder
local inputs = {"hello", " ", "world"}

-- a table to hold the output
-- after a compression-decompression
-- to be matched against the initial
-- `inputs' table above
local outputs = {}

--[[ start of encoding ]]
do
    -- create a writer stream
    -- 
    -- tip: always check for errors
    local ok, writer_stream = pcall(
        function()
            return xz.stream.writer(xz.PRESET_DEFAULT, xz.check.CRC64)
        end
    )

    -- an error occurred ?
    if (not ok) then
        -- raise the error
        error(writer_stream)
    end

    -- open / create a destination file to hold the compressed content
    local output_file = assert(
        io.open("lua-xz.xz", "wb"),
        "failed to open the file lua-xz.xz for writing"
    )

    -- define a producer function
    -- to feed uncompressed data
    -- to be encoded by the writer stream
    local inputs_start = 1
    local function producer()
        local element
        if (inputs_start <= #inputs) then
            element = inputs[inputs_start]
            inputs_start = inputs_start + 1
        end
        return element
    end

    -- define a consumer function
    -- to handle compressed chunks
    -- emitted by the writer stream
    local function consumer(compressed_chunk)
        output_file:write(compressed_chunk)
    end

    do
        -- execute the stream
        -- 
        -- tip: always check for errors
        local ok, exec_err = pcall(
            function()
                writer_stream:exec(producer, consumer)
            end
        )

        -- an error occurred ?
        if (not ok) then
            -- raise the error
            error(exec_err)
        end
    end

    -- close the writer stream to free resources
    -- 
    -- tip: it is automatically freed on garbage collection
    writer_stream:close()

    -- close the output file
    output_file:close()
end
--[[ end of encoding]]

--[[ start of decoding ]]
do
    -- create a reader stream
    -- 
    -- tip: always check for errors
    local ok, reader_stream = pcall(
        function()
            return xz.stream.reader(xz.MEMLIMIT_UNLIMITED, xz.CONCATENATED)
        end
    )

    -- open the file created above
    -- to feed the reader stream
    -- with content to decode
    local input_file = assert(
        io.open("lua-xz.xz", "rb"),
        "failed to open the file lua-xz.xz for reading"
    )

    -- define a producer function
    -- to feed compressed data
    -- to be decoded by the reader stream
    local function producer()

        -- define the number of bytes
        -- to be read from the input file
        -- in a single chunk.
        -- In a real world scenario,
        -- 8kb (8 * 1024) would be
        -- a reasonable value
        local chunk_size = 64

        -- read the chunk from file
        local chunk = input_file:read(chunk_size)

        -- return the chunk read
        return chunk
    end

    -- define a consumer function
    -- to handle decompressed chunks
    -- emitted by the reader stream
    local function consumer(decompressed_chunk)
        table.insert(outputs, decompressed_chunk)
    end

    do
        -- execute the stream
        -- 
        -- tip: always check for errors
        local ok, exec_err = pcall(
            function()
                reader_stream:exec(producer, consumer)
            end
        )
    
        -- an error occurred ?
        if (not ok) then
            -- raise the error
            error(exec_err)
        end
    end

    -- close the reader stream to free resources
    -- 
    -- tip: it is automatically freed on garbage collection
    reader_stream:close()

    -- close the file
    input_file:close()
end
--[[ end of decoding]]

-- make sure that the decoded data
-- after compression-decompression
-- matches the initial inputs
assert(
    table.concat(inputs) == table.concat(outputs),
    "compression-decompression mismatch: the final output did not match the initial input"
)
```

## Constants

| Key | Type | Description |
|---|---|---|
| version | string | Version of the binding (e.g.: 0.0.1) |
| _VERSION | string | Version of the underlying ```liblzma``` library (e.g.: 5.8.1) |
| MEMLIMIT_UNLIMITED | integer | A custom value in Lua to disable a memory limit |
| CONCATENATED | integer | Flag to enable decoding of concatenated files |
| PRESET_DEFAULT | integer | Default compression preset |
| check.NONE | integer | No integrity check is calculated |
| check.CRC32 | integer | Calculate CRC32 integrity check using the polynomial from the IEEE 802.3 standard |
| check.CRC64 | integer | Calculate CRC64 integrity check using the polynomial from the ECMA-182 standard |
| check.SHA256 | integer | SHA256 integrity check |

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
* *Signature*: ```xz.stream.reader(memlimit, flags)```
* *Parameters*: 
    * *memlimit* (```integer```): Memory usage limit as bytes. Use ```xz.MEMLIMIT_UNLIMITED``` to effectively disable the limiter;
    * *flags* (```integer```): Bitwise-or of zero or more of the decoder flags (for now, only ```xz.CONCATENATED``` is provided as constant);
* *Return* (```userdata```): An instance of the stream reader class.

#### Instance methods

##### close

* *Description*: Closes the reader stream and free resources
* *Signature*: ```stream:close()```
    * *stream* (```userdata```): An instance of the stream class;
    * *Return* (```void```): Nothing.

##### exec

* *Description*: Feeds data to be decompressed
* *Signature*: ```stream:exec(producer, consumer [, buffersize ])```
    * *stream* (```userdata```): An instance of the stream class;
    * *Parameters*: 
        * *data* (```string```): The data to feed the stream;
        * *buffersize* (```integer | nil```): The size in bytes of the output buffer to allocate memory at stream execution. If no value is provided, it uses the value of ```LUA_XZ_BUFFER_SIZE``` from the [lua-xz.h](./src/lua-xz.h) header file. **Note**: choosing larger values for this parameter makes decompression faster, at a price of higher memory consumption;
    * *Return* (```void```)

### stream (writer)

A stream to compress data to .xz format

#### Static methods

##### writer

* *Description*: Creates a writer stream to compress data to .xz format
* *Signature*: ```xz.stream.writer(preset, check [, buffersize ])```
* *Parameters*: 
    * *preset* (```integer | string```): Compression level as an integer [0, 9] or string with a single digit [0-9] occasionally followed by 'e' character to indicate extreme compression preset. Valid values: 0, 1, 2, 6, 7, 9, "3", "5", "7", "5e", "6e", "8e" and so on;
    * *check* (```integer```): Type of the integrity check to calculate from uncompressed data;
    * *buffersize* (```integer | nil```): The size of the output buffer to allocate memory at stream creation. If no value is provided, it uses the value of ```LUA_XZ_BUFFER_SIZE``` from the [lua-xz.h](./src/lua-xz.h) header file;
* *Return* (```userdata```): An instance of the stream writer class.

#### Instance methods

##### close

* *Description*: Closes the writer stream and free resources
* *Signature*: ```stream:close()```
    * *stream* (```userdata```): An instance of the stream class;
    * *Return* (```void```): Nothing.

##### exec

* *Description*: Feeds data to be compressed
* *Signature*: ```stream:exec(producer, consumer [, buffersize ])```
    * *stream* (```userdata```): An instance of the stream class;
    * *Parameters*: 
        * *data* (```string```): The data to feed the stream;
        * *buffersize* (```integer | nil```): The size in bytes of the output buffer to allocate memory at stream execution. If no value is provided, it uses the value of ```LUA_XZ_BUFFER_SIZE``` from the [lua-xz.h](./src/lua-xz.h) header file. **Note**: choosing larger values for this parameter makes compression faster, at a price of higher memory consumption;
    * *Return* (```void```)

## Known limitations

1. Due the fact that ```lua-xz``` depends on the ```liblzma``` library, which internally uses 64-bit integers on most of its APIs, ```lua-xz``` requires a Lua build that uses 64-bit integers. This means that the Lua type ```lua_Integer``` must be able to store at least 64 bits;
2. Again due the dependency on ```liblzma```, which requires a C compiler compliant with C99, ```lua-xz``` also requires C99 to build.

## Change log

* v0.0.1: Initial release

## Future works

* Add CMake as build system
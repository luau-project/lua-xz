-- load the library
local xz = require("lua-xz")

-- the file to decompress
local filename = "README.md.lzma"

-- the decompressed file name
local decompressed_filename = "lzma-copy-of-" .. (filename:gsub("%.lzma$", ""))

-- create a lzma reader stream
-- 
-- first parameter:
--  a memory limit in bytes
--  can be chosen
-- 
-- note: xz.MEMLIMIT_UNLIMITED does not use a limit
-- 
-- tip: always check for errors
local ok, stream = pcall(
    function()
        return xz.stream.lzmareader(xz.MEMLIMIT_UNLIMITED)
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
-- to be decoded by the lzma reader stream
local function producer()

    -- define the number of bytes
    -- to be read from the input file
    -- in a single chunk (8 kb).
    local chunk_size = 8 * 1024

    -- read the chunk from file
    local chunk = input:read(chunk_size)

    -- return the chunk read
    return chunk
end

-- define a consumer function
-- to handle decompressed chunks
-- emitted by the lzma reader stream
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

-- close the lzma reader stream to free resources
-- 
-- tip: it is automatically freed on garbage collection
stream:close()

-- close the output file
output:close()

-- close the input file
input:close()
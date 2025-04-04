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
--  xz.CHECK_CRC32 instead.
-- 
-- tip: always check for errors
local ok, stream = pcall(
    function()
        return xz.stream.writer(xz.PRESET_DEFAULT, xz.CHECK_CRC64)
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
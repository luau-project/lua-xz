-- load the library
local xz = require("lua-xz")

-- the file to compress
local filename = assert(arg[1], "You must provide the file name to the tar file from the linux kernel to the script")

-- compressed file name
local compressed_filename = filename .. ".xz"

-- create a xz writer stream
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
--  2) check:
--      if the .xz file needs to be
--      decompressed with XZ Embedded, use
--      xz.check.CRC32 instead.
-- 
-- tip: always check for errors
local ok, stream = pcall(
    function()
        local check = xz.check.supported(xz.check.CRC64) and xz.check.CRC64 or xz.check.CRC32
        return xz.stream.xzwriter(xz.PRESET_DEFAULT, check)
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
-- to be encoded by the xz writer stream
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
-- to handle compressed chunks
-- emitted by the xz writer stream
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
        -- close the stream
        stream:close()

        -- close files
        output:close()
        input:close()

        -- raise the error
        error(exec_err)
    end
end

-- close the xz writer stream to free resources
-- 
-- tip: it is automatically freed on garbage collection
stream:close()

-- close the output file
output:close()

-- close the input file
input:close()

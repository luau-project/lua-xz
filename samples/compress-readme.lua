-- load the library
local xz = require("lua-xz")

-- the file to compress
local filename = "README.md"

-- compressed file name
local compressed_filename = filename .. ".xz"

-- create a writer stream
-- 
-- first parameter:
--  the compression preset
-- second parameter:
--  the integrity check
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

-- define the number of bytes
-- of the chunk to be read
-- from the input file.
-- In a real world scenario,
-- 8kb (8 * 1024) would be
-- a reasonable value
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
    -- 
    -- tip: always check for errors
    ok, compressed_chunk = pcall(
        function()
            return stream:update(chunk)
        end
    )

    -- an error occurred ?
    if (not ok) then
        -- raise the error
        error(compressed_chunk)
    end

    -- write the compressed chunk
    -- to the output file
    output:write(compressed_chunk)

    -- read the next chunk from the input file
    chunk = input:read(chunk_size)
end

-- finish the stream and get
-- the last compressed chunk
-- 
-- tip: always check for errors
ok, compressed_chunk = pcall(
    function()
        return stream:finish()
    end
)

-- an error occurred ?
if (not ok) then
    -- raise the error
    error(compressed_chunk)
end

-- close the writer stream to free resources
-- 
-- tip: it is automatically freed on garbage collection
stream:close()

-- write the compressed chunk
-- to the output file
output:write(compressed_chunk)

-- close the output file
output:close()

-- close the input file
input:close()
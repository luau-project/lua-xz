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
-- the decompressed chunk from the
-- reader stream
local decompressed_chunk

-- keep reading the file
-- while there is data to read
while (chunk ~= nil) do
    -- feed the stream with the chunk
    -- and get the decompressed chunk
    -- 
    -- tip: always check for errors
    ok, decompressed_chunk = pcall(
        function()
            return stream:update(chunk)
        end
    )

    -- an error occurred ?
    if (not ok) then
        -- raise the error
        error(decompressed_chunk)
    end

    -- write the decompressed chunk
    -- to the output file
    output:write(decompressed_chunk)

    -- read the next chunk from the input file
    chunk = input:read(chunk_size)
end

-- finish the stream and get
-- the last decompressed chunk
-- 
-- tip: always check for errors
ok, decompressed_chunk = pcall(
    function()
        return stream:finish()
    end
)

-- an error occurred ?
if (not ok) then
    -- raise the error
    error(decompressed_chunk)
end

-- close the reader stream to free resources
-- 
-- tip: it is automatically freed on garbage collection
stream:close()

-- write the decompressed chunk
-- to the output file
output:write(decompressed_chunk)

-- close the output file
output:close()

-- close the input file
input:close()
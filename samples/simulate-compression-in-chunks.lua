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
            return xz.stream.writer(xz.PRESET_DEFAULT, xz.CHECK_CRC64)
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

    -- save a variable to hold each compressed_chunk
    local compressed_chunk

    for _, IN in ipairs(inputs) do
        -- feed the writer stream with the IN chunk
        -- and get the compressed chunk
        -- 
        -- tip: always check for errors
        ok, compressed_chunk = pcall(
            function()
                return writer_stream:update(IN)
            end
        )

        -- an error occurred ?
        if (not ok) then
            -- raise the error
            error(compressed_chunk)
        end

        -- write the compressed chunk to the output file
        output_file:write(compressed_chunk)
    end

    -- get the last compressed chunk
    -- 
    -- tip: always check for errors
    ok, compressed_chunk = pcall(
        function()
            return writer_stream:finish()
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
    writer_stream:close()

    -- write the last compressed chunk to the output file
    output_file:write(compressed_chunk)

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

    -- define the number of bytes
    -- of the chunk to be read
    -- from the input file.
    -- In a real world scenario,
    -- 8kb (8 * 1024) would be
    -- a reasonable value
    local chunk_size = 64

    -- read first chunk
    local OUT = input_file:read(chunk_size)

    -- save a variable to hold each decompressed_chunk
    local decompressed_chunk

    -- keep reading the file while
    -- the chunk is not nil
    while (OUT ~= nil) do
        -- feed the reader stream stream with the OUT chunk
        -- and get the decompressed chunk
        -- 
        -- tip: always check for errors
        ok, decompressed_chunk = pcall(
            function()
                return reader_stream:update(OUT)
            end
        )

        -- an error occurred ?
        if (not ok) then
            -- raise the error
            error(decompressed_chunk)
        end

        -- save the decompressed chunk
        -- in the `outputs' table
        table.insert(outputs, decompressed_chunk)

        -- read the next chunk from the file
        OUT = input_file:read(chunk_size)
    end

    -- grab the last decompressed chunk
    -- 
    -- tip: always check for errors
    ok, decompressed_chunk = pcall(
        function()
            return reader_stream:finish()
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
    reader_stream:close()
    
    -- save the last decompressed chunk
    -- in the `outputs' table
    table.insert(outputs, decompressed_chunk)

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
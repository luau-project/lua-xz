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
    -- create a lzma writer stream
    -- 
    -- tip: always check for errors
    local ok, writer_stream = pcall(
        function()
            return xz.stream.lzmawriter(xz.PRESET_DEFAULT)
        end
    )

    -- an error occurred ?
    if (not ok) then
        -- raise the error
        error(writer_stream)
    end

    -- open / create a destination file to hold the compressed content
    local output_file = assert(
        io.open("lua-xz.lzma", "wb"),
        "failed to open the file lua-xz.lzma for writing"
    )

    -- define a producer function
    -- to feed uncompressed data
    -- to be encoded by the lzma writer stream
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
    -- emitted by the lzma writer stream
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
            -- close the stream
            writer_stream:close()

            -- close files
            output_file:close()

            -- raise the error
            error(exec_err)
        end
    end

    -- close the lzma writer stream to free resources
    -- 
    -- tip: it is automatically freed on garbage collection
    writer_stream:close()

    -- close the output file
    output_file:close()
end
--[[ end of encoding]]

--[[ start of decoding ]]
do
    -- create a lzma reader stream
    -- 
    -- tip: always check for errors
    local ok, reader_stream = pcall(
        function()
            return xz.stream.lzmareader(xz.MEMLIMIT_UNLIMITED)
        end
    )

    -- open the file created above
    -- to feed the lzma reader stream
    -- with content to decode
    local input_file = assert(
        io.open("lua-xz.lzma", "rb"),
        "failed to open the file lua-xz.lzma for reading"
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
        local chunk = input_file:read(chunk_size)

        -- return the chunk read
        return chunk
    end

    -- define a consumer function
    -- to handle decompressed chunks
    -- emitted by the lzma reader stream
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
            -- close the stream
            reader_stream:close()

            -- close files
            input_file:close()

            -- raise the error
            error(exec_err)
        end
    end

    -- close the lzma reader stream to free resources
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